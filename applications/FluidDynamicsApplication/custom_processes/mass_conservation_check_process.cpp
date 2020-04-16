//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Simon Wenczowski
//
//

// System includes

// External includes

// Project includes
#include "utilities/openmp_utils.h"
#include "processes/find_nodal_h_process.h"
#include "custom_utilities/fluid_element_utilities.h"

// Application includes
#include "mass_conservation_check_process.h"


namespace Kratos
{

/* Public functions *******************************************************/

/// constructor
MassConservationCheckProcess::MassConservationCheckProcess(
        ModelPart& rModelPart,
        const int CorrectionFreq)
    : Process(),
      mrModelPart(rModelPart),
      mCorrectionFreq(CorrectionFreq) {

}



/// constructor (direct input of settings)
MassConservationCheckProcess::MassConservationCheckProcess(
    ModelPart& rModelPart,
    Parameters& rParameters)
    : Process(), mrModelPart(rModelPart) {

    const Parameters default_parameters = GetDefaultParameters();

    rParameters.ValidateAndAssignDefaults(default_parameters);

    mCorrectionFreq = rParameters["correction_frequency_in_time_steps"].GetInt();
}



/// Initialization function to find the initial volumes and print first lines in the log-file
std::string MassConservationCheckProcess::Initialize(){

    double pos_vol = 0.0;
    double neg_vol = 0.0;
    double inter_area = 0.0;
    mrModelPart.GetCommunicator().SynchronizeVariable(DISTANCE);
    mrModelPart.GetCommunicator().SynchronizeVariable(VELOCITY);
    mrModelPart.GetCommunicator().SynchronizeVariable(MESH_VELOCITY);

    ComputeVolumesAndInterface( pos_vol, neg_vol, inter_area );

    this->mInitialPositiveVolume = pos_vol;
    this->mInitialNegativeVolume = neg_vol;
    this->mTheoreticalNegativeVolume = neg_vol;

    std::string output_line =   "------ Initial values ----------------- \n";
    output_line +=              "  positive volume (air)   = " + std::to_string(this->mInitialPositiveVolume) + "\n";
    output_line +=              "  negative volume (water) = " + std::to_string(this->mInitialNegativeVolume) + "\n";
    output_line +=              "  fluid interface (area)  = " + std::to_string(inter_area) + "\n\n";
    output_line +=              "------ Time step values --------------- \n";
    output_line +=              "sim_time \t\twater_vol \t\tair_vol \t\twater_err \t\tnet_inf \t\t";
    output_line +=              "net_outf \t\tint_area \n";

    return output_line;
}



std::string MassConservationCheckProcess::ComputeBalancedVolume(){

    // performing all necessary calculations
    double pos_vol = 0.0;
    double neg_vol = 0.0;
    double inter_area = 0.0;
    mrModelPart.GetCommunicator().SynchronizeVariable(DISTANCE);
    mrModelPart.GetCommunicator().SynchronizeVariable(VELOCITY);
    mrModelPart.GetCommunicator().SynchronizeVariable(MESH_VELOCITY);

    ComputeVolumesAndInterface( pos_vol, neg_vol, inter_area );
    double net_inflow_inlet = ComputeFlowOverBoundary(INLET);
    double net_inflow_outlet = ComputeFlowOverBoundary(OUTLET);
    mInterfaceArea = inter_area;

    // making a "time step forwards" and updating the
    const double current_time = mrModelPart.GetProcessInfo()[TIME];
    const double current_dt = mrModelPart.GetProcessInfo()[DELTA_TIME];

    mQNet0 = net_inflow_inlet + net_inflow_outlet;
    mTheoreticalNegativeVolume += current_dt * mQNet0;


    mWaterVolumeError = mTheoreticalNegativeVolume - neg_vol;

    // assembly of the log message
    std::string output_line_timestep =  std::to_string(current_time) + "\t\t";
    output_line_timestep +=             std::to_string(neg_vol) + "\t\t";
    output_line_timestep +=             std::to_string(pos_vol) + "\t\t";
    output_line_timestep +=             std::to_string(mWaterVolumeError) + "\t\t";
    output_line_timestep +=             std::to_string(net_inflow_inlet) + "\t\t";
    output_line_timestep +=             std::to_string(net_inflow_outlet) + "\t\t";
    output_line_timestep +=             std::to_string(inter_area) + "\n";

    return output_line_timestep;
}



double MassConservationCheckProcess::ComputeDtForConvection(){

    // a small step is set to avoid numerical problems
    double time_step_for_convection = 1.0e-7;
    const auto& r_comm = mrModelPart.GetCommunicator();

    if ( mWaterVolumeError > 0.0 ){
        // case: water volume was lost by mistake
        mAddWater = true;
        double water_outflow_over_boundary = OrthogonalFlowIntoAir( 1.0 );
        if ( r_comm.GetDataCommunicator().SumAll( water_outflow_over_boundary ) ){
            // checking if flow is sufficient (avoid division by 0)
            if ( water_outflow_over_boundary > 1.0e-7 ){
                time_step_for_convection = mWaterVolumeError / water_outflow_over_boundary;
            }
        } else {
            KRATOS_ERROR << "Communication failed in MassConservationCheckProcess::ComputeDtForConvection()";
        }
    }
    else if ( mWaterVolumeError < 0.0 ){
        // case: water volume was gained by mistake
        mAddWater = false;
        double water_inflow_over_boundary = OrthogonalFlowIntoAir( -1.0 );
        if ( r_comm.GetDataCommunicator().SumAll( water_inflow_over_boundary ) ){
            // checking if flow is sufficient (avoid division by 0)
            if ( water_inflow_over_boundary > 1.0e-7 ){
                time_step_for_convection = - mWaterVolumeError / water_inflow_over_boundary;
            }
        } else {
            KRATOS_ERROR << "Communication failed in MassConservationCheckProcess::ComputeDtForConvection()";
        }
    }
    else {
        // case: Exactly the correct volume of water is present
        mAddWater = true;
    }

    KRATOS_WARNING_IF("MassConservationCheckProcess", time_step_for_convection < 0.0) << "A time step smaller than 0.0 was computed." << std::endl;

    return time_step_for_convection;
}



void MassConservationCheckProcess::ApplyLocalCorrection( Variable<double>& rAuxDistVar ){

    if (mrModelPart.GetProcessInfo()[STEP] % mCorrectionFreq == 0 ){
        const auto node_begin = mrModelPart.GetCommunicator().LocalMesh().NodesBegin();
        const int number_nodes = static_cast<int>(mrModelPart.GetCommunicator().LocalMesh().NumberOfNodes());

        for (int i_node = 0; i_node < number_nodes; ++i_node){
            auto it_node = node_begin + i_node;
            double& r_original_dist = it_node->FastGetSolutionStepValue( DISTANCE, 0 );
            const double& r_aux_dist = it_node->GetValue( rAuxDistVar );

            if ( mAddWater ){
                // choosing minimum to extend water domain
                r_original_dist = std::min( r_original_dist, r_aux_dist );
            } else {
                // choosing maximum to extend water domain
                r_original_dist = std::max( r_original_dist, r_aux_dist );
            }
        }
    }
}



void MassConservationCheckProcess::ReCheckTheMassConservation(){

    // recomputation based on the new distance field.
    double pos_vol = 0.0;
    double neg_vol = 0.0;
    double inter_area = 0.0;
    ComputeVolumesAndInterface( pos_vol, neg_vol, inter_area );

    mWaterVolumeError = mTheoreticalNegativeVolume - neg_vol;
    mInterfaceArea = inter_area;
}



void MassConservationCheckProcess::ApplyGlobalCorrection(){

    const double inter_area = mInterfaceArea;

    // check if it is time for a correction (if wished for)
    if (mrModelPart.GetProcessInfo()[STEP] % mCorrectionFreq == 0 && inter_area > 1e-7){
        // if water is missing, a shift into negative direction increases the water volume
        const double shift_for_correction = - mWaterVolumeError / inter_area;

        ShiftDistanceField( shift_for_correction );
    }

}



void MassConservationCheckProcess::ComputeVolumesAndInterface( double& rPositiveVolume, double& rNegativeVolume, double& rInterfaceArea ){

    // initalisation (necessary because no reduction for type reference)
    double pos_vol = 0.0;
    double neg_vol = 0.0;
    double int_area = 0.0;

    #pragma omp parallel for reduction(+: pos_vol, neg_vol, int_area)
    for (int i_elem = 0; i_elem < static_cast<int>(mrModelPart.NumberOfElements()); ++i_elem){
        // iteration over all elements
        const auto it_elem = mrModelPart.ElementsBegin() + i_elem;

        Matrix shape_functions;
        GeometryType::ShapeFunctionsGradientsType shape_derivatives;

        const auto rGeom = it_elem->GetGeometry();
        unsigned int pt_count_pos = 0;
        unsigned int pt_count_neg = 0;
        const bool is_geometry_cut = IsGeometryCut(rGeom, pt_count_neg, pt_count_pos);

        if ( pt_count_pos == rGeom.PointsNumber() ){
            // all nodes are positive (pointer is necessary to maintain polymorphism of DomainSize())
            pos_vol += it_elem->pGetGeometry()->DomainSize();
        }
        else if ( pt_count_neg == rGeom.PointsNumber() ){
            // all nodes are negative (pointer is necessary to maintain polymorphism of DomainSize())
            neg_vol += it_elem->pGetGeometry()->DomainSize();
        }
        else if ( is_geometry_cut ){
            // element is cut by the surface (splitting)
            Kratos::unique_ptr<ModifiedShapeFunctions> p_modified_sh_func = nullptr;
            Vector w_gauss_pos_side(3, 0.0);
            Vector w_gauss_neg_side(3, 0.0);
            Vector w_gauss_interface(3, 0.0);

            Vector nodal_distances( rGeom.PointsNumber(), 0.0 );
            for (unsigned int i = 0; i < rGeom.PointsNumber(); i++){
                // Control mechanism to avoid 0.0 ( is necessary because "distance_modification" possibly not yet executed )
                if ( rGeom[i].FastGetSolutionStepValue(DISTANCE) == 0.0 ){
                    it_elem->GetGeometry().GetPoint(i).FastGetSolutionStepValue(DISTANCE) = -1.0e-7;
                }
                nodal_distances[i] = rGeom[i].FastGetSolutionStepValue(DISTANCE);
            }

            if ( rGeom.PointsNumber() == 3 ){ p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>(it_elem->pGetGeometry(), nodal_distances); }
            else if ( rGeom.PointsNumber() == 4 ){ p_modified_sh_func = Kratos::make_unique<Tetrahedra3D4ModifiedShapeFunctions>(it_elem->pGetGeometry(), nodal_distances); }
            else { KRATOS_ERROR << "The process can not be applied on this kind of element" << std::endl; }

            // Call the positive side modified shape functions calculator (Gauss weights woulb be enough)
            // Object p_modified_sh_func has full knowledge of slit geometry
            p_modified_sh_func->ComputePositiveSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_pos_side,                   // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration (1 point per triangle)

            for ( unsigned int i = 0; i < w_gauss_pos_side.size(); i++){
                pos_vol += w_gauss_pos_side[i];
            }

            // Call the negative side modified shape functions calculator
            // Object p_modified_sh_func has full knowledge of slit geometry
            p_modified_sh_func->ComputeNegativeSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_neg_side,                   // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration

            for ( unsigned int i = 0; i < w_gauss_neg_side.size(); i++){
                neg_vol += w_gauss_neg_side[i];
            }

            // Concerning their area, the positive and negative side of the interface are equal
            p_modified_sh_func->ComputeInterfacePositiveSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_interface,                  // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration

            for ( unsigned int i = 0; i < w_gauss_interface.size(); i++){
                int_area += std::abs( w_gauss_interface[i] );
            }
        }
    }
    mrModelPart.GetCommunicator().SynchronizeCurrentDataToMin(DISTANCE);
    rPositiveVolume = mrModelPart.GetCommunicator().GetDataCommunicator().SumAll(pos_vol);
    rNegativeVolume = mrModelPart.GetCommunicator().GetDataCommunicator().SumAll(neg_vol);
    rInterfaceArea = mrModelPart.GetCommunicator().GetDataCommunicator().SumAll(int_area);
}



double MassConservationCheckProcess::OrthogonalFlowIntoAir( const double Factor )
{
    double outflow = 0.0;

    KRATOS_ERROR_IF( std::abs( Factor ) != 1.0 ) << "MassConservationPocess: Given value of argument 'factor' is not plausible. Consider '1' or '-1'." << std::endl;

    #pragma omp parallel for reduction(+: outflow)
    for (int i_elem = 0; i_elem < static_cast<int>(mrModelPart.NumberOfElements()); ++i_elem){
        // iteration over all elements
        const auto it_elem = mrModelPart.ElementsBegin() + i_elem;

        Matrix shape_functions;
        GeometryType::ShapeFunctionsGradientsType shape_derivatives;

        const auto r_geom = it_elem->GetGeometry();
        unsigned int pt_count_pos = 0;
        unsigned int pt_count_neg = 0;
        const bool geom_is_cut = IsGeometryCut(r_geom, pt_count_neg, pt_count_pos);
        if (geom_is_cut){
            // element is cut by the surface (splitting)
            Kratos::unique_ptr<ModifiedShapeFunctions> p_modified_sh_func = nullptr;
            Vector w_gauss_interface(3, 0.0);

            Vector Distance( r_geom.PointsNumber(), 0.0 );
            for (unsigned int i = 0; i < r_geom.PointsNumber(); i++){
                // Control mechanism to avoid 0.0 ( is necessary because "distance_modification" possibly not yet executed )
                if ( r_geom[i].FastGetSolutionStepValue(DISTANCE) == 0.0 ){
                    it_elem->GetGeometry().GetPoint(i).FastGetSolutionStepValue(DISTANCE) = -1.0e-7;
                }
                Distance[i] = r_geom[i].FastGetSolutionStepValue(DISTANCE);
            }

            if ( r_geom.PointsNumber() == 3 ){ p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else if ( r_geom.PointsNumber() == 4 ){ p_modified_sh_func = Kratos::make_unique<Tetrahedra3D4ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else { KRATOS_ERROR << "The process can not be applied on this kind of element" << std::endl; }

            // Concerning their area, the positive and negative side of the interface are equal
            p_modified_sh_func->ComputeInterfacePositiveSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_interface,                  // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_2);          // second order Gauss integration

            // negative side outwards area normal vector values for the Gauss pts. of given quadrature
            std::vector<Vector> normal_vectors;
            p_modified_sh_func->ComputeNegativeSideInterfaceAreaNormals(
                normal_vectors,
                GeometryData::GI_GAUSS_2
            );

            // iteration over all 3 or 6 integration points
            for ( unsigned int i_gauss = 0; i_gauss < w_gauss_interface.size(); i_gauss++)
            {
                const double& r_weight = w_gauss_interface[i_gauss];
                const auto& r_N = row(shape_functions, i_gauss);

                auto& r_normal = normal_vectors[i_gauss];
                r_normal /= norm_2( r_normal );
                r_normal *= Factor;

                array_1d<double,3> interpolated_velocity = ZeroVector(3);
                for (unsigned int n_node = 0; n_node < r_geom.PointsNumber(); n_node++){
                    noalias( interpolated_velocity ) += r_N[n_node] * (r_geom[n_node].FastGetSolutionStepValue(VELOCITY)
                        - r_geom[n_node].FastGetSolutionStepValue(MESH_VELOCITY));
                }

                const double& r_orthogonal_flow = inner_prod( r_normal, interpolated_velocity );
                // checking if it is really an outflow towards the air domain
                if ( 0.0 < r_orthogonal_flow ){
                    outflow += r_weight * r_orthogonal_flow;
                }
            }
        }
    }
    mrModelPart.GetCommunicator().SynchronizeCurrentDataToMin(DISTANCE);
    const double global_outflow = mrModelPart.GetCommunicator().GetDataCommunicator().SumAll(outflow);
    return global_outflow;
}



double MassConservationCheckProcess::ComputeInterfaceArea(){

    double int_area = 0.0;

    #pragma omp parallel for reduction(+: int_area)
    for (int i_elem = 0; i_elem < static_cast<int>(mrModelPart.NumberOfElements()); ++i_elem){
        // iteration over all elements
        const auto it_elem = mrModelPart.ElementsBegin() + i_elem;

        Matrix shape_functions;
        GeometryType::ShapeFunctionsGradientsType shape_derivatives;

        const auto rGeom = it_elem->GetGeometry();
        unsigned int pt_count_pos = 0;
        unsigned int pt_count_neg = 0;
        const bool geom_is_cut = IsGeometryCut(rGeom, pt_count_pos, pt_count_neg);
        if (geom_is_cut) {
            // element is cut by the surface (splitting)
            Kratos::unique_ptr<ModifiedShapeFunctions> p_modified_sh_func = nullptr;
            Vector w_gauss_interface(3, 0.0);

            Vector Distance( rGeom.PointsNumber(), 0.0 );
            for (unsigned int i = 0; i < rGeom.PointsNumber(); i++){
                Distance[i] = rGeom[i].FastGetSolutionStepValue(DISTANCE);
            }

            if ( rGeom.PointsNumber() == 3 ){ p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else if ( rGeom.PointsNumber() == 4 ){ p_modified_sh_func = Kratos::make_unique<Tetrahedra3D4ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else { KRATOS_ERROR << "The process can not be applied on this kind of element" << std::endl; }

            // Concerning their area, the positive and negative side of the interface are equal
            p_modified_sh_func->ComputeInterfacePositiveSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_interface,                  // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration

            for ( unsigned int i = 0; i < w_gauss_interface.size(); i++){
                int_area += std::abs( w_gauss_interface[i] );
            }
        }
    }

    return int_area;
}



double MassConservationCheckProcess::ComputeNegativeVolume(){

    double neg_vol = 0.0;

    #pragma omp parallel for reduction(+: neg_vol)
    for (int i_elem = 0; i_elem < static_cast<int>(mrModelPart.NumberOfElements()); ++i_elem){
        // iteration over all elements
        const auto it_elem = mrModelPart.ElementsBegin() + i_elem;

        Matrix shape_functions;
        GeometryType::ShapeFunctionsGradientsType shape_derivatives;
        const auto rGeom = it_elem->GetGeometry();
        unsigned int pt_count_pos = 0;
        unsigned int pt_count_neg = 0;
        const bool is_geometry_cut = IsGeometryCut(rGeom, pt_count_neg, pt_count_pos);

        if ( pt_count_pos == rGeom.PointsNumber() ){
            // jump into next iteration
            continue;
        }
        else if ( pt_count_neg == rGeom.PointsNumber() ){
            // all nodes are negative
            neg_vol += it_elem->pGetGeometry()->DomainSize();
        }
        else if (is_geometry_cut) {
            // element is cut by the surface (splitting)
            Kratos::unique_ptr<ModifiedShapeFunctions> p_modified_sh_func = nullptr;
            Vector w_gauss_neg_side(3, 0.0);

            Vector Distance( rGeom.PointsNumber(), 0.0 );
            for (unsigned int i = 0; i < rGeom.PointsNumber(); i++){
                Distance[i] = rGeom[i].FastGetSolutionStepValue(DISTANCE);
            }

            if ( rGeom.PointsNumber() == 3 ){ p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else if ( rGeom.PointsNumber() == 4 ){ p_modified_sh_func = Kratos::make_unique<Tetrahedra3D4ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else { KRATOS_ERROR << "The process can not be applied on this kind of element" << std::endl; }

            // Call the negative side modified shape functions calculator
            // Object p_modified_sh_func has full knowledge of slit geometry
            p_modified_sh_func->ComputeNegativeSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_neg_side,                   // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration

            for ( unsigned int i = 0; i < w_gauss_neg_side.size(); i++){
                neg_vol += w_gauss_neg_side[i];
            }
        }
    }

    return neg_vol;
}



double MassConservationCheckProcess::ComputePositiveVolume(){

    double pos_vol = 0.0;

    #pragma omp parallel for reduction(+: pos_vol)
    for (int i_elem = 0; i_elem < static_cast<int>(mrModelPart.NumberOfElements()); ++i_elem){
        // iteration over all elements
        const auto it_elem = mrModelPart.ElementsBegin() + i_elem;

        Matrix shape_functions;
        GeometryType::ShapeFunctionsGradientsType shape_derivatives;
        const auto rGeom = it_elem->GetGeometry();
        unsigned int pt_count_pos = 0;
        unsigned int pt_count_neg = 0;

        // instead of using data.isCut()
        const bool is_geometry_cut = IsGeometryCut(rGeom, pt_count_neg, pt_count_pos);

        if ( pt_count_pos == rGeom.PointsNumber() ){
            // all nodes are positive
            pos_vol += it_elem->pGetGeometry()->DomainSize();;
        }
        else if ( pt_count_neg == rGeom.PointsNumber() ){
            // jump into the next iteration
            continue;
        }
        else if (is_geometry_cut) {
            // element is cut by the surface (splitting)
            Kratos::unique_ptr<ModifiedShapeFunctions> p_modified_sh_func = nullptr;
            Vector w_gauss_pos_side(3, 0.0);

            Vector Distance( rGeom.PointsNumber(), 0.0 );
            for (unsigned int i = 0; i < rGeom.PointsNumber(); i++){
                Distance[i] = rGeom[i].FastGetSolutionStepValue(DISTANCE);
            }

            if ( rGeom.PointsNumber() == 3 ){ p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else if ( rGeom.PointsNumber() == 4 ){ p_modified_sh_func = Kratos::make_unique<Tetrahedra3D4ModifiedShapeFunctions>(it_elem->pGetGeometry(), Distance); }
            else { KRATOS_ERROR << "The process can not be applied on this kind of element" << std::endl; }

            // Call the positive side modified shape functions calculator (Gauss weights woulb be enough)
            // Object p_modified_sh_func has full knowledge of slit geometry
            p_modified_sh_func->ComputePositiveSideShapeFunctionsAndGradientsValues(
                    shape_functions,                    // N
                    shape_derivatives,                  // DN
                    w_gauss_pos_side,                   // includes the weights of the GAUSS points (!!!)
                    GeometryData::GI_GAUSS_1);          // first order Gauss integration (1 point per triangle)

            for ( unsigned int i = 0; i < w_gauss_pos_side.size(); i++){
                pos_vol += w_gauss_pos_side[i];
            }
        }
    }

    return pos_vol;
}



double MassConservationCheckProcess::ComputeFlowOverBoundary( const Kratos::Flags BoundaryFlag ){

    // Convention: "mass" is considered as "water", meaning the volumes with a negative distance is considered
    double inflow_over_boundary = 0.0;
    const double epsilon = 1.0e-8;

    #pragma omp parallel for reduction(+: inflow_over_boundary)
    for (int i_cond = 0; i_cond < static_cast<int>(mrModelPart.NumberOfConditions()); ++i_cond){

        // iteration over all conditions (pointer to condition)
        const auto p_condition = mrModelPart.ConditionsBegin() + i_cond;

        if ( p_condition->Is( BoundaryFlag ) ){

            const auto& rGeom = p_condition->GetGeometry();
            const int dim = rGeom.PointsNumber();
            Vector distance( rGeom.PointsNumber(), 0.0 );

            unsigned int neg_count = 0;
            unsigned int pos_count = 0;
            const bool is_geometry_cut = IsGeometryCut(rGeom, neg_count, pos_count);

            // leave the current iteration of the condition is completely on positive side
            if ( pos_count == rGeom.PointsNumber() ){ continue; }

            if (dim == 2){      // 2D case: condition is a line

                array_1d<double, 3> normal;
                this->CalculateNormal2D( normal, rGeom );
                if( norm_2( normal ) < epsilon ){ continue; }
                else { normal /= norm_2( normal ); }

                // --- the condition is completely on the negative side (2D)
                if ( neg_count == rGeom.PointsNumber() ){
                    const auto& IntegrationPoints = rGeom.IntegrationPoints(GeometryData::GI_GAUSS_2);
                    const unsigned int num_gauss = IntegrationPoints.size();
                    Vector gauss_pts_det_jabobian = ZeroVector(num_gauss);
                    rGeom.DeterminantOfJacobian(gauss_pts_det_jabobian, GeometryData::GI_GAUSS_2);
                    const Matrix n_container = rGeom.ShapeFunctionsValues( GeometryData::GI_GAUSS_2 );

                    for (unsigned int i_gauss = 0; i_gauss < num_gauss; i_gauss++){
                        const auto& N = row(n_container, i_gauss);
                        double const w_gauss = gauss_pts_det_jabobian[i_gauss] * IntegrationPoints[i_gauss].Weight();
                        array_1d<double,3> interpolated_velocity = ZeroVector(3);
                        for (unsigned int n_node = 0; n_node < rGeom.PointsNumber(); n_node++){
                            noalias( interpolated_velocity ) += N[n_node] * (rGeom[n_node].FastGetSolutionStepValue(VELOCITY) -
                             rGeom[n_node].FastGetSolutionStepValue(MESH_VELOCITY)) ;
                        }
                        inflow_over_boundary -= w_gauss * inner_prod( normal, interpolated_velocity );
                    }

                // --- the condition is cut (2D)
                } else if (is_geometry_cut){

                    array_1d<double, 3> aux_velocity1, aux_velocity2;

                    // Generation of an auxiliary line that only covers the negative fraction ("transferring the splitting mechanism to 1D")
                    Line3D2<IndexedPoint>::Pointer p_aux_line = nullptr;
                    for (unsigned int i = 0; i < rGeom.PointsNumber(); i++) {
                        distance[i] = p_condition->GetGeometry()[i].FastGetSolutionStepValue( DISTANCE );
                    }
                    GenerateAuxLine( rGeom, distance, p_aux_line, aux_velocity1, aux_velocity2 );

                    // Gauss point information for auxiliary line geometry
                    const auto& IntegrationPoints = p_aux_line->IntegrationPoints( GeometryData::GI_GAUSS_2 );
                    const unsigned int num_gauss = IntegrationPoints.size();
                    Vector gauss_pts_det_jabobian = ZeroVector(num_gauss);
                    p_aux_line->DeterminantOfJacobian(gauss_pts_det_jabobian, GeometryData::GI_GAUSS_2);
                    const Matrix n_container = p_aux_line->ShapeFunctionsValues( GeometryData::GI_GAUSS_2 );

                    for (unsigned int i_gauss = 0; i_gauss < num_gauss; i_gauss++){
                        const auto& N = row(n_container, i_gauss);
                        double const w_gauss = gauss_pts_det_jabobian[i_gauss] * IntegrationPoints[i_gauss].Weight();
                        const array_1d<double,3> interpolatedVelocity = N[0] * aux_velocity1 + N[1] * aux_velocity2;
                        inflow_over_boundary -= std::abs( w_gauss ) * inner_prod( normal, interpolatedVelocity );
                    }
                }
            }

            else if (dim == 3){
                // 3D case: condition is a triangle (implemented routines can be used)

                array_1d<double, 3> normal;
                this->CalculateNormal3D( normal, rGeom);
                if( norm_2( normal ) < epsilon ){ continue; }
                else { normal /= norm_2( normal ); }

                // --- the condition is completely on the negative side (3D)
                if ( neg_count == rGeom.PointsNumber() ){

                    const GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(GeometryData::GI_GAUSS_2);
                    const unsigned int num_gauss = IntegrationPoints.size();
                    Vector gauss_pts_det_jabobian = ZeroVector(num_gauss);
                    rGeom.DeterminantOfJacobian(gauss_pts_det_jabobian, GeometryData::GI_GAUSS_2);
                    const Matrix n_container = rGeom.ShapeFunctionsValues( GeometryData::GI_GAUSS_2 );

                    for (unsigned int i_gauss = 0; i_gauss < num_gauss; i_gauss++){
                        const auto& N = row(n_container, i_gauss);
                        double const wGauss = gauss_pts_det_jabobian[i_gauss] * IntegrationPoints[i_gauss].Weight();
                        array_1d<double,3> interpolated_velocity = ZeroVector(3);
                        for (unsigned int n_node = 0; n_node < rGeom.PointsNumber(); n_node++){
                            noalias( interpolated_velocity ) += N[n_node] * (rGeom[n_node].FastGetSolutionStepValue(VELOCITY)
                                - rGeom[n_node].FastGetSolutionStepValue(MESH_VELOCITY));
                        }
                        inflow_over_boundary -= wGauss * inner_prod( normal, interpolated_velocity );
                    }

                // --- the condition is cut
                } else if ( is_geometry_cut ){

                    Matrix r_shape_functions;
                    GeometryType::ShapeFunctionsGradientsType r_shape_derivatives;
                    Vector w_gauss_neg_side;

                    for (unsigned int i = 0; i < rGeom.PointsNumber(); i++) {
                        distance[i] = p_condition->GetGeometry()[i].FastGetSolutionStepValue( DISTANCE );
                    }

                    // generating an auxiliary Triangle2D3 geometry the "Triangle2D3ModifiedShapeFunctions" can work with
                    const auto aux_2D_triangle = GenerateAuxTriangle( rGeom );
                    // passing the auxiliary triangle
                    const auto p_modified_sh_func = Kratos::make_unique<Triangle2D3ModifiedShapeFunctions>( aux_2D_triangle, distance);

                    p_modified_sh_func->ComputeNegativeSideShapeFunctionsAndGradientsValues(
                        r_shape_functions,                  // N
                        r_shape_derivatives,                // DN
                        w_gauss_neg_side,                   // includes the weights of the GAUSS points (!!!)
                        GeometryData::GI_GAUSS_2);          // second order Gauss integration

                    auto i_points = aux_2D_triangle->IntegrationPoints( GeometryData::GI_GAUSS_2 );

                    // interating velocity over the negative area of the condition
                    for ( unsigned int i_gauss = 0; i_gauss < w_gauss_neg_side.size(); i_gauss++){
                        const array_1d<double,3>& N = row(r_shape_functions, i_gauss);
                        array_1d<double,3> interpolated_velocity = ZeroVector(3);
                        for (unsigned int n_node = 0; n_node < rGeom.PointsNumber(); n_node++){
                            noalias( interpolated_velocity ) += N[n_node] * rGeom[n_node].FastGetSolutionStepValue(VELOCITY);
                        }
                        // abs() is necessary because the auxiliary Triangle2D3 geometry could possibly be inverted
                        // the normal still comes from the oiginal triangle
                        inflow_over_boundary -= std::abs( w_gauss_neg_side[i_gauss] ) * inner_prod( normal, interpolated_velocity );
                    }
                }
            }
        }
    }
    const double global_inflow_over_boundary = mrModelPart.GetCommunicator().GetDataCommunicator().SumAll(inflow_over_boundary);
    return global_inflow_over_boundary;
}



void MassConservationCheckProcess::ShiftDistanceField( const double DeltaDist ){

    // negative shift = "more water"
    // positive shift = "less water"
    ModelPart::NodesContainerType rNodes = mrModelPart.Nodes();
    #pragma omp parallel for
    for(int count = 0; count < static_cast<int>(rNodes.size()); count++){
        ModelPart::NodesContainerType::iterator i_node = rNodes.begin() + count;
        i_node->FastGetSolutionStepValue( DISTANCE ) += DeltaDist;
    }
}



void MassConservationCheckProcess::CalculateNormal2D(array_1d<double,3>& An, const Geometry<Node<3> >& pGeometry){

    An[0] =   pGeometry[1].Y() - pGeometry[0].Y();
    An[1] = - (pGeometry[1].X() - pGeometry[0].X());
    An[2] =    0.0;
}



void MassConservationCheckProcess::CalculateNormal3D(array_1d<double,3>& An, const Geometry<Node<3> >& pGeometry){

    array_1d<double,3> v1,v2;
    v1[0] = pGeometry[1].X() - pGeometry[0].X();
    v1[1] = pGeometry[1].Y() - pGeometry[0].Y();
    v1[2] = pGeometry[1].Z() - pGeometry[0].Z();

    v2[0] = pGeometry[2].X() - pGeometry[0].X();
    v2[1] = pGeometry[2].Y() - pGeometry[0].Y();
    v2[2] = pGeometry[2].Z() - pGeometry[0].Z();

    MathUtils<double>::CrossProduct(An,v1,v2);
    An *= 0.5;
}



/// Function to convert Triangle3D3N into Triangle2D3N which can be handled by the splitting utilitity
Triangle2D3<Node<3>>::Pointer MassConservationCheckProcess::GenerateAuxTriangle( const Geometry<Node<3> >& rGeom ){

    // Generating auxiliary "Triangle2D3" because the original geometry is "Triangle3D3"

    // vectors that will form the rows of the rotation matrix
    array_1d<double,3> vec_u = rGeom[1].Coordinates() - rGeom[0].Coordinates();
    vec_u /= norm_2( vec_u );

    array_1d<double,3> vec_w;
    MathUtils<double>::CrossProduct(vec_w, vec_u, ( rGeom[2].Coordinates() - rGeom[0].Coordinates() ) );
    vec_w /= norm_2( vec_w );

    array_1d<double,3> vec_v;
    MathUtils<double>::CrossProduct(vec_v, vec_u, vec_w );

    // assembly of the rotation matrix
    Matrix rot_mat = ZeroMatrix(3,3);
    for (unsigned int i = 0; i < 3; i++){
        rot_mat(0,i) = vec_u[i];
        rot_mat(1,i) = vec_v[i];
        rot_mat(2,i) = vec_w[i];
    }

    // rotating the original geometry into a position parallel to the x-y plane by applying the rotation matrix
    // the z-coordinates are then equal and will be neglected afterwards (check is performed before)
    array_1d<double,3> coord1_transformed = prod( rot_mat, rGeom[0].Coordinates() );
    array_1d<double,3> coord2_transformed = prod( rot_mat, rGeom[1].Coordinates() );
    array_1d<double,3> coord3_transformed = prod( rot_mat, rGeom[2].Coordinates() );
    KRATOS_DEBUG_ERROR_IF_NOT( std::abs(coord1_transformed[2] - coord2_transformed[2])<1.0e-7 &&
                            std::abs(coord1_transformed[2] - coord3_transformed[2])<1.0e-7 );

    // creating auxiliary nodes based on the transformed position
    Node<3UL>::Pointer node1 = Kratos::make_intrusive<Kratos::Node<3UL>>( mrModelPart.Nodes().size() + 2, coord1_transformed[0], coord1_transformed[1] );
    Node<3UL>::Pointer node2 = Kratos::make_intrusive<Kratos::Node<3UL>>( mrModelPart.Nodes().size() + 3, coord2_transformed[0], coord2_transformed[1] );
    Node<3UL>::Pointer node3 = Kratos::make_intrusive<Kratos::Node<3UL>>( mrModelPart.Nodes().size() + 4, coord3_transformed[0], coord3_transformed[1] );

    // finally creating the desired Triangle2D3 based on the nodes
    Triangle2D3<Node<3>>::Pointer aux_2D_triangle = Kratos::make_shared< Triangle2D3<Node<3> > >( node1, node2, node3 );
    return aux_2D_triangle;
}



void MassConservationCheckProcess::GenerateAuxLine( const Geometry<Node<3> >& rGeom,
                                                    const Vector& rDistance,
                                                    Line3D2<IndexedPoint>::Pointer& rpAuxLine,
                                                    array_1d<double, 3>& rAuxVelocity1,
                                                    array_1d<double, 3>& rAuxVelocity2 ){

    // Compute the relative coordinate of the intersection point over the edge
    const double aux_node_rel_location = std::abs ( rDistance[0] /( rDistance[1]-rDistance[0] ));
    // Shape function values at the position where the surface cuts the element
    Vector n_cut = ZeroVector(2);
    n_cut[0] = 1.0 - aux_node_rel_location;
    n_cut[1] = aux_node_rel_location;

    // Creation of an auxiliary geometry which covers the negative volume domain only
    // (imitation of the splitting mechanism for triangles)
    PointerVectorSet<IndexedPoint, IndexedObject> aux_point_container;
    aux_point_container.reserve(2);
    array_1d<double, 3> aux_point1_coords, aux_point2_coords;

    IndexedPoint::Pointer paux_point1 = nullptr;
    IndexedPoint::Pointer paux_point2 = nullptr;
    // Modify the node with the positive distance
    for (unsigned int i_node = 0; i_node < rGeom.PointsNumber(); i_node++){
        if ( rGeom[i_node].FastGetSolutionStepValue( DISTANCE ) > 0.0 ){
            // interpolating position for the new node
            aux_point1_coords[0] = n_cut[0] * rGeom[0].X() + n_cut[1] * rGeom[1].X();
            aux_point1_coords[1] = n_cut[0] * rGeom[0].Y() + n_cut[1] * rGeom[1].Y();
            aux_point1_coords[2] = n_cut[0] * rGeom[0].Z() + n_cut[1] * rGeom[1].Z();
            paux_point1 = Kratos::make_shared<IndexedPoint>(aux_point1_coords, mrModelPart.NumberOfNodes()+1);
            rAuxVelocity1 = n_cut[0] * rGeom[0].FastGetSolutionStepValue( VELOCITY ) + n_cut[1] * rGeom[1].FastGetSolutionStepValue( VELOCITY );
        } else {
            aux_point2_coords[0] = rGeom[i_node].X();
            aux_point2_coords[1] = rGeom[i_node].Y();
            aux_point2_coords[2] = rGeom[i_node].Z();
            paux_point2 = Kratos::make_shared<IndexedPoint>(aux_point2_coords, mrModelPart.NumberOfNodes()+2);
            rAuxVelocity2 = rGeom[i_node].FastGetSolutionStepValue( VELOCITY );
        }
    }

    rpAuxLine = Kratos::make_shared< Line3D2 < IndexedPoint > >( paux_point1, paux_point2 );
}

const Parameters MassConservationCheckProcess::GetDefaultParameters()
{
    const Parameters default_parameters = Parameters(R"(
    {
        "correction_frequency_in_time_steps"     : 20
    })" );
    return default_parameters;
}

bool MassConservationCheckProcess::IsGeometryCut(
    const GeometryType &rGeometry,
    unsigned int &PtCountNeg,
    unsigned int &PtCountPos)
{
    PtCountNeg = 0;
    PtCountPos = 0;
    for (unsigned int pt = 0; pt < rGeometry.Points().size(); pt++){
        if ( rGeometry[pt].FastGetSolutionStepValue(DISTANCE) > 0.0 ){
            PtCountPos++;
        } else {
            PtCountNeg++;
        }
    }
    return (PtCountNeg > 0 && PtCountPos > 0);
}


};  // namespace Kratos.