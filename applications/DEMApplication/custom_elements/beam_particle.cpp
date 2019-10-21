//
// Author: Joaquín Irazábal jirazabal@cimne.upc.edu
//

// System includes
#include <string>
#include <iostream>
#include <cmath>

#include <fstream>

// External includes

// Project includes
#include "beam_particle.h"

namespace Kratos {

    BeamParticle::BeamParticle() : SphericContinuumParticle() {}

    BeamParticle::BeamParticle(IndexType NewId, GeometryType::Pointer pGeometry) : SphericContinuumParticle(NewId, pGeometry) {}

    BeamParticle::BeamParticle(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties) : SphericContinuumParticle(NewId, pGeometry, pProperties) {}

    BeamParticle::BeamParticle(IndexType NewId, NodesArrayType const& ThisNodes) : SphericContinuumParticle(NewId, ThisNodes) {}

    BeamParticle::BeamParticle(Element::Pointer p_continuum_spheric_particle)
    {
        GeometryType::Pointer p_geom = p_continuum_spheric_particle->pGetGeometry();
        PropertiesType::Pointer pProperties = p_continuum_spheric_particle->pGetProperties();
        BeamParticle(p_continuum_spheric_particle->Id(), p_geom, pProperties);
    }

    BeamParticle& BeamParticle::operator=(const BeamParticle& rOther) {

        SphericParticle::operator=(rOther);

        mNeighbourContactRadius = rOther.mNeighbourContactRadius;
        mNeighbourIndentation = rOther.mNeighbourIndentation;
        mNeighbourTgOfFriAng = rOther.mNeighbourTgOfFriAng;
        mNeighbourContactStress = rOther.mNeighbourContactStress;
        mNeighbourCohesion = rOther.mNeighbourCohesion;
        mNeighbourRigidContactRadius = rOther.mNeighbourRigidContactRadius;
        mNeighbourRigidIndentation = rOther.mNeighbourRigidIndentation;
        mNeighbourRigidTgOfFriAng = rOther.mNeighbourRigidTgOfFriAng;
        mNeighbourRigidContactStress = rOther.mNeighbourRigidContactStress;
        mNeighbourRigidCohesion = rOther.mNeighbourRigidCohesion;

        return *this;
    }

    Element::Pointer BeamParticle::Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
    {
        GeometryType::Pointer p_geom = GetGeometry().Create(ThisNodes);

        return Element::Pointer(new BeamParticle(NewId, p_geom, pProperties));
    }

    void BeamParticle::Initialize(const ProcessInfo& r_process_info)
    {
        SphericContinuumParticle::Initialize(r_process_info);

        double distance = GetProperties()[BEAM_DISTANCE];

        if (distance)
        {
            double contact_area = GetProperties()[BEAM_CROSS_SECTION];
            if (IsSkin()) distance *= 0.5;

            GetGeometry()[0].FastGetSolutionStepValue(REPRESENTATIVE_VOLUME) = distance * contact_area;
            SetMass(GetDensity() * distance * contact_area);

            if (this->Is(DEMFlags::HAS_ROTATION)) {
                // double aux2 = sqrt( 12.0 * GetProperties()[BEAM_PLANAR_MOMENT_OF_INERTIA_X] / contact_area );
                // double aux3 = sqrt( 12.0 * GetProperties()[BEAM_PLANAR_MOMENT_OF_INERTIA_Y] / contact_area );

                // GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[0] = 0.083333333333333333 * GetDensity() * distance * contact_area * (aux1 * aux1 + aux2 * aux2);
                // GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[1] = 0.083333333333333333 * GetDensity() * distance * contact_area * (aux2 * aux2 + aux3 * aux3);
                // GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[2] = 0.083333333333333333 * GetDensity() * distance * contact_area * (aux1 * aux1 + aux3 * aux3);

                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[0] = GetProperties()[BEAM_PRINCIPAL_MOMENTS_OF_INERTIA_X] * GetDensity() * distance * contact_area;
                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[1] = GetProperties()[BEAM_PRINCIPAL_MOMENTS_OF_INERTIA_Y] * GetDensity() * distance * contact_area;
                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[2] = GetProperties()[BEAM_PRINCIPAL_MOMENTS_OF_INERTIA_Z] * GetDensity() * distance * contact_area;
            }
        }
        else
        {
            if (this->Is(DEMFlags::HAS_ROTATION)) {
                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[0] = GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_MOMENT_OF_INERTIA);
                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[1] = GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_MOMENT_OF_INERTIA);
                GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[2] = GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_MOMENT_OF_INERTIA);
            }
        }
    }

    void BeamParticle::InitializeSolutionStep(ProcessInfo& r_process_info)
    {
        mRadius = this->GetGeometry()[0].FastGetSolutionStepValue(RADIUS); //Just in case someone is overwriting the radius in Python
        mPartialRepresentativeVolume = 0.0;
        double& elastic_energy = this->GetElasticEnergy();
        elastic_energy = 0.0;
        if (this->Is(DEMFlags::HAS_STRESS_TENSOR)) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    (*mStressTensor)(i,j) = 0.0;
                }
            }
        }
    }

    void BeamParticle::CalculateLocalAngularMomentum(array_1d<double, 3>& r_angular_momentum) {}

    void BeamParticle::ComputeNewNeighboursHistoricalData(DenseVector<int>& temp_neighbours_ids, std::vector<array_1d<double, 3> >& temp_neighbour_elastic_contact_forces)
    {
        std::vector<array_1d<double, 3> > temp_neighbour_elastic_extra_contact_forces;
        std::vector<double> temp_neighbour_contact_radius;
        std::vector<double> temp_neighbour_indentation;
        std::vector<double> temp_neighbour_tg_of_fri_ang;
        std::vector<double> temp_neighbour_contact_stress;
        std::vector<double> temp_neighbour_cohesion;
        unsigned int new_size = mNeighbourElements.size();
        array_1d<double, 3> vector_of_zeros = ZeroVector(3);
        temp_neighbours_ids.resize(new_size, false);
        temp_neighbour_elastic_contact_forces.resize(new_size);
        temp_neighbour_elastic_extra_contact_forces.resize(new_size);
        temp_neighbour_contact_radius.resize(new_size);
        temp_neighbour_indentation.resize(new_size);
        temp_neighbour_tg_of_fri_ang.resize(new_size);
        temp_neighbour_contact_stress.resize(new_size);
        temp_neighbour_cohesion.resize(new_size);

        DenseVector<int>& vector_of_ids_of_neighbours = GetValue(NEIGHBOUR_IDS);

        for (unsigned int i = 0; i < new_size; i++) {
            noalias(temp_neighbour_elastic_contact_forces[i]) = vector_of_zeros;
            noalias(temp_neighbour_elastic_extra_contact_forces[i]) = vector_of_zeros;
            temp_neighbour_contact_radius[i] = 0.0;
            temp_neighbour_indentation[i] = 0.0;
            temp_neighbour_tg_of_fri_ang[i] = 1e20;
            temp_neighbour_contact_stress[i] = 0.0;
            temp_neighbour_cohesion[i] = 0.0;

            if (mNeighbourElements[i] == NULL) { // This is required by the continuum sphere which reorders the neighbors
                temp_neighbours_ids[i] = -1;
                continue;
            }

            temp_neighbours_ids[i] = mNeighbourElements[i]->Id();

            for (unsigned int j = 0; j < vector_of_ids_of_neighbours.size(); j++) {
                if (int(temp_neighbours_ids[i]) == vector_of_ids_of_neighbours[j] && vector_of_ids_of_neighbours[j] != -1) {
                    noalias(temp_neighbour_elastic_contact_forces[i]) = mNeighbourElasticContactForces[j];
                    noalias(temp_neighbour_elastic_extra_contact_forces[i]) = mNeighbourElasticExtraContactForces[j]; //TODO: remove this from discontinuum!!
                    temp_neighbour_contact_radius[i] = mNeighbourContactRadius[j];
                    temp_neighbour_indentation[i] = mNeighbourIndentation[j];
                    temp_neighbour_tg_of_fri_ang[i] = mNeighbourTgOfFriAng[j];
                    temp_neighbour_contact_stress[i] = mNeighbourContactStress[j];
                    temp_neighbour_cohesion[i] = mNeighbourCohesion[j];
                    break;
                }
            }
        }

        vector_of_ids_of_neighbours.swap(temp_neighbours_ids);
        mNeighbourElasticContactForces.swap(temp_neighbour_elastic_contact_forces);
        mNeighbourElasticExtraContactForces.swap(temp_neighbour_elastic_extra_contact_forces);
        mNeighbourContactRadius.swap(temp_neighbour_contact_radius);
        mNeighbourIndentation.swap(temp_neighbour_indentation);
        mNeighbourTgOfFriAng.swap(temp_neighbour_tg_of_fri_ang);
        mNeighbourContactStress.swap(temp_neighbour_contact_stress);
        mNeighbourCohesion.swap(temp_neighbour_cohesion);
    }

    void BeamParticle::ComputeNewRigidFaceNeighboursHistoricalData()
    {
        array_1d<double, 3> vector_of_zeros = ZeroVector(3);
        std::vector<DEMWall*>& rNeighbours = this->mNeighbourRigidFaces;
        unsigned int new_size              = rNeighbours.size();
        std::vector<int> temp_neighbours_ids(new_size); //these two temporal vectors are very small, saving them as a member of the particle loses time (usually they consist on 1 member).
        std::vector<array_1d<double, 3> > temp_neighbours_elastic_contact_forces(new_size);
        std::vector<array_1d<double, 3> > temp_neighbours_contact_forces(new_size);
        std::vector<double> temp_contact_radius(new_size);
        std::vector<double> temp_indentation(new_size);
        std::vector<double> temp_tg_of_fri_ang(new_size);
        std::vector<double> temp_contact_stress(new_size);
        std::vector<double> temp_cohesion(new_size);

        for (unsigned int i = 0; i<rNeighbours.size(); i++){

            noalias(temp_neighbours_elastic_contact_forces[i]) = vector_of_zeros;
            noalias(temp_neighbours_contact_forces[i]) = vector_of_zeros;
            temp_contact_radius[i] = 0.0;
            temp_indentation[i] = 0.0;
            temp_tg_of_fri_ang[i] = 1e20;
            temp_contact_stress[i] = 0.0;
            temp_cohesion[i] = 0.0;

            if (rNeighbours[i] == NULL) { // This is required by the continuum sphere which reorders the neighbors
                temp_neighbours_ids[i] = -1;
                continue;
            }

            temp_neighbours_ids[i] = static_cast<int>(rNeighbours[i]->Id());

            for (unsigned int j = 0; j != mFemOldNeighbourIds.size(); j++) {
                if (static_cast<int>(temp_neighbours_ids[i]) == mFemOldNeighbourIds[j] && mFemOldNeighbourIds[j] != -1) {
                    noalias(temp_neighbours_elastic_contact_forces[i]) = mNeighbourRigidFacesElasticContactForce[j];
                    noalias(temp_neighbours_contact_forces[i]) = mNeighbourRigidFacesTotalContactForce[j];
                    temp_contact_radius[i] = mNeighbourRigidContactRadius[j];
                    temp_indentation[i] = mNeighbourRigidIndentation[j];
                    temp_tg_of_fri_ang[i] = mNeighbourRigidTgOfFriAng[j];
                    temp_contact_stress[i] = mNeighbourRigidContactStress[j];
                    temp_cohesion[i] =  mNeighbourRigidCohesion[j];
                    break;
                }
            }
        }

        mFemOldNeighbourIds.swap(temp_neighbours_ids);
        mNeighbourRigidFacesElasticContactForce.swap(temp_neighbours_elastic_contact_forces);
        mNeighbourRigidFacesTotalContactForce.swap(temp_neighbours_contact_forces);
        mNeighbourRigidContactRadius.swap(temp_contact_radius);
        mNeighbourRigidIndentation.swap(temp_indentation);
        mNeighbourRigidTgOfFriAng.swap(temp_tg_of_fri_ang);
        mNeighbourRigidContactStress.swap(temp_contact_stress);
        mNeighbourRigidCohesion.swap(temp_cohesion);
    }

    void BeamParticle::ComputeRollingFriction(array_1d<double, 3>& rolling_resistance_moment, double& RollingResistance, double dt) {}

    void BeamParticle::ContactAreaWeighting() {}

    void BeamParticle::ComputeBallToBallContactForce(SphericParticle::ParticleDataBuffer & data_buffer,
                                                     ProcessInfo& r_process_info,
                                                     array_1d<double, 3>& rElasticForce,
                                                     array_1d<double, 3>& rContactForce,
                                                     double& RollingResistance)
    {
        NodeType& this_node = this->GetGeometry()[0];
        DEM_COPY_SECOND_TO_FIRST_3(data_buffer.mMyCoors, this_node)

        const int time_steps = r_process_info[TIME_STEPS];
        const int& search_control = r_process_info[SEARCH_CONTROL];
        DenseVector<int>& search_control_vector = r_process_info[SEARCH_CONTROL_VECTOR];

        const array_1d<double, 3>& vel         = this->GetGeometry()[0].FastGetSolutionStepValue(VELOCITY);
        const array_1d<double, 3>& delta_displ = this->GetGeometry()[0].FastGetSolutionStepValue(DELTA_DISPLACEMENT);
        const array_1d<double, 3>& ang_vel     = this->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY);
        int NeighbourSize = mNeighbourElements.size();
        GetGeometry()[0].GetSolutionStepValue(NEIGHBOUR_SIZE) = NeighbourSize;

        for (int i = 0; data_buffer.SetNextNeighbourOrExit(i); ++i) {

            if (mNeighbourElements[i] == NULL) continue;
            if (this->Is(NEW_ENTITY) && mNeighbourElements[i]->Is(NEW_ENTITY)) continue;
            SphericContinuumParticle* neighbour_iterator = dynamic_cast<SphericContinuumParticle*>(mNeighbourElements[i]);
            data_buffer.mpOtherParticle = neighbour_iterator;

            unsigned int neighbour_iterator_id = data_buffer.mpOtherParticle->Id();

            noalias(data_buffer.mOtherToMeVector) = this->GetGeometry()[0].Coordinates() - data_buffer.mpOtherParticle->GetGeometry()[0].Coordinates();

            const double& other_radius = data_buffer.mpOtherParticle->GetRadius();

            data_buffer.mDistance = DEM_MODULUS_3(data_buffer.mOtherToMeVector);
            double radius_sum = GetRadius() + other_radius;

            double initial_delta = GetInitialDelta(i);

            double initial_dist = radius_sum - initial_delta;
            double indentation = initial_dist - data_buffer.mDistance;
            double myYoung = GetYoung();
            double myPoisson = GetPoisson();

            double kn_el = 0.0;
            double kt_el_0 = 0.0;
            double kt_el_1 = 0.0;
            double DeltDisp[3] = {0.0};
            double RelVel[3] = {0.0};
            DEM_SET_COMPONENTS_TO_ZERO_3x3(data_buffer.mLocalCoordSystem)
            DEM_SET_COMPONENTS_TO_ZERO_3x3(data_buffer.mOldLocalCoordSystem)
            bool sliding = false;

            double contact_tau = 0.0;
            double contact_sigma = 0.0;
            double failure_criterion_state = 0.0;
            double acumulated_damage = 0.0;

            // Getting neighbor properties
            double other_young = data_buffer.mpOtherParticle->GetYoung();
            double other_poisson = data_buffer.mpOtherParticle->GetPoisson();
            double equiv_poisson;
            if ((myPoisson + other_poisson) != 0.0) { equiv_poisson = 2.0 * myPoisson * other_poisson / (myPoisson + other_poisson); }
            else { equiv_poisson = 0.0; }

            double equiv_young = 2.0 * myYoung * other_young / (myYoung + other_young);
            double calculation_area = 0.0;
            const double equiv_shear = equiv_young / (2.0 * (1 + equiv_poisson));

            if (i < (int)mContinuumInitialNeighborsSize) {
                double area = this->GetProperties()[BEAM_CROSS_SECTION];
                double other_area = data_buffer.mpOtherParticle->GetProperties()[BEAM_CROSS_SECTION];
                calculation_area = std::max(area, other_area);
                mContinuumConstitutiveLawArray[i]->CalculateElasticConstants(kn_el, kt_el_0, kt_el_1, initial_dist, equiv_young, equiv_poisson, calculation_area, this, neighbour_iterator);
            }

            EvaluateDeltaDisplacement(data_buffer, DeltDisp, RelVel, data_buffer.mLocalCoordSystem, data_buffer.mOldLocalCoordSystem, vel, delta_displ);

            if (this->Is(DEMFlags::HAS_ROTATION)) {
                RelativeDisplacementAndVelocityOfContactPointDueToRotationQuaternion(DeltDisp, RelVel, data_buffer.mOldLocalCoordSystem, other_radius, data_buffer.mDt, ang_vel, neighbour_iterator);
            }

            RelativeDisplacementAndVelocityOfContactPointDueToOtherReasons(r_process_info, DeltDisp, RelVel, data_buffer.mOldLocalCoordSystem, data_buffer.mLocalCoordSystem, neighbour_iterator);

            double LocalDeltDisp[3] = {0.0};
            double LocalElasticContactForce[3] = {0.0}; // 0: first tangential, // 1: second tangential, // 2: normal force
            double LocalElasticExtraContactForce[3] = {0.0};
            double GlobalElasticContactForce[3] = {0.0};
            double GlobalElasticExtraContactForce[3] = {0.0};
            double TotalGlobalElasticContactForce[3] = {0.0};
            double OldLocalElasticContactForce[3] = {0.0};

            // FilterNonSignificantDisplacements(DeltDisp, RelVel, indentation);

            GeometryFunctions::VectorGlobal2Local(data_buffer.mLocalCoordSystem, DeltDisp, LocalDeltDisp);

            RotateOldContactForces(data_buffer.mOldLocalCoordSystem, data_buffer.mLocalCoordSystem, mNeighbourElasticContactForces[i]);
            RotateOldContactForces(data_buffer.mOldLocalCoordSystem, data_buffer.mLocalCoordSystem, mNeighbourElasticExtraContactForces[i]);

            GeometryFunctions::VectorGlobal2Local(data_buffer.mLocalCoordSystem, mNeighbourElasticContactForces[i], OldLocalElasticContactForce);

            GlobalElasticContactForce[0] = mNeighbourElasticContactForces[i][0];
            GlobalElasticContactForce[1] = mNeighbourElasticContactForces[i][1];
            GlobalElasticContactForce[2] = mNeighbourElasticContactForces[i][2];

            GeometryFunctions::VectorGlobal2Local(data_buffer.mLocalCoordSystem, GlobalElasticContactForce, LocalElasticContactForce); //TODO: can we remove this? We should overwrite LocalElasticContactForce afterwards

            double ViscoDampingLocalContactForce[3] = {0.0};
            double equiv_visco_damp_coeff_normal;
            double equiv_visco_damp_coeff_tangential_0;
            double equiv_visco_damp_coeff_tangential_1;
            double ElasticLocalRotationalMoment[3] = {0.0};
            double ViscoLocalRotationalMoment[3] = {0.0};
            double cohesive_force =  0.0;
            double LocalRelVel[3] = {0.0};
            GeometryFunctions::VectorGlobal2Local(data_buffer.mLocalCoordSystem, RelVel, LocalRelVel);

            if (i < (int)mContinuumInitialNeighborsSize) {

                mContinuumConstitutiveLawArray[i]->CalculateForces(r_process_info,
                                                                   OldLocalElasticContactForce,
                                                                   LocalElasticContactForce,
                                                                   LocalElasticExtraContactForce,
                                                                   data_buffer.mLocalCoordSystem,
                                                                   LocalDeltDisp,
                                                                   kn_el,
                                                                   kt_el_0,
                                                                   kt_el_1,
                                                                   contact_sigma,
                                                                   contact_tau,
                                                                   failure_criterion_state,
                                                                   equiv_young,
                                                                   equiv_shear,
                                                                   indentation,
                                                                   calculation_area,
                                                                   acumulated_damage,
                                                                   this,
                                                                   neighbour_iterator,
                                                                   i,
                                                                   r_process_info[TIME_STEPS],
                                                                   sliding,
                                                                   search_control,
                                                                   search_control_vector,
                                                                   equiv_visco_damp_coeff_normal,
                                                                   equiv_visco_damp_coeff_tangential_0,
                                                                   equiv_visco_damp_coeff_tangential_1,
                                                                   LocalRelVel,
                                                                   ViscoDampingLocalContactForce);

            } else if (indentation > 0.0) {
                const double previous_indentation = indentation + LocalDeltDisp[2];
                mDiscontinuumConstitutiveLaw->CalculateForces(r_process_info,
                                                              OldLocalElasticContactForce,
                                                              LocalElasticContactForce,
                                                              LocalDeltDisp,
                                                              LocalRelVel,
                                                              indentation,
                                                              previous_indentation,
                                                              ViscoDampingLocalContactForce,
                                                              cohesive_force,
                                                              this,
                                                              data_buffer.mpOtherParticle,
                                                              sliding,
                                                              data_buffer.mLocalCoordSystem);
            } else { //Not bonded and no idata_buffer.mpOtherParticlendentation
                LocalElasticContactForce[0] = 0.0; LocalElasticContactForce[1] = 0.0; LocalElasticContactForce[2] = 0.0;
                ViscoDampingLocalContactForce[0] = 0.0; ViscoDampingLocalContactForce[1] = 0.0; ViscoDampingLocalContactForce[2] = 0.0;
                cohesive_force= 0.0;
            }

            // Transforming to global forces and adding up
            double LocalContactForce[3] = {0.0};
            double GlobalContactForce[3] = {0.0};

            if (this->Is(DEMFlags::HAS_STRESS_TENSOR) && (i < (int)mContinuumInitialNeighborsSize)) { // We leave apart the discontinuum neighbors (the same for the walls). The neighbor would not be able to do the same if we activate it.
                mContinuumConstitutiveLawArray[i]->AddPoissonContribution(equiv_poisson, data_buffer.mLocalCoordSystem, LocalElasticContactForce[2], calculation_area, mSymmStressTensor, this, neighbour_iterator, r_process_info, i, indentation);
            }

            array_1d<double, 3> other_ball_to_ball_forces(3,0.0);
            ComputeOtherBallToBallForces(other_ball_to_ball_forces);

            AddUpForcesAndProject(data_buffer.mOldLocalCoordSystem, data_buffer.mLocalCoordSystem, LocalContactForce, LocalElasticContactForce, LocalElasticExtraContactForce, GlobalContactForce,
                                  GlobalElasticContactForce, GlobalElasticExtraContactForce, TotalGlobalElasticContactForce,ViscoDampingLocalContactForce, 0.0, other_ball_to_ball_forces, rElasticForce, rContactForce, i, r_process_info); //TODO: replace the 0.0 with an actual cohesive force for discontinuum neighbours

            if (this->Is(DEMFlags::HAS_ROTATION)) {
                ComputeMoments(LocalContactForce[2], TotalGlobalElasticContactForce, RollingResistance, data_buffer.mLocalCoordSystem[2], data_buffer.mpOtherParticle, indentation);
                if (i < (int)mContinuumInitialNeighborsSize && mIniNeighbourFailureId[i] == 0) {
                    mContinuumConstitutiveLawArray[i]->ComputeParticleRotationalMoments(this, neighbour_iterator, equiv_young, data_buffer.mDistance, calculation_area,
                                                                                        data_buffer.mLocalCoordSystem, ElasticLocalRotationalMoment, ViscoLocalRotationalMoment, equiv_poisson, indentation);
                }

                AddUpMomentsAndProject(data_buffer.mLocalCoordSystem, ElasticLocalRotationalMoment, ViscoLocalRotationalMoment);
            }

            if (r_process_info[CONTACT_MESH_OPTION] == 1 && (i < (int)mContinuumInitialNeighborsSize) && this->Id() < neighbour_iterator_id) {
                double total_local_elastic_contact_force[3] = {0.0};
                total_local_elastic_contact_force[0] = LocalElasticContactForce[0] + LocalElasticExtraContactForce[0];
                total_local_elastic_contact_force[1] = LocalElasticContactForce[1] + LocalElasticExtraContactForce[1];
                total_local_elastic_contact_force[2] = LocalElasticContactForce[2] + LocalElasticExtraContactForce[2];
                CalculateOnContinuumContactElements(i, total_local_elastic_contact_force, contact_sigma, contact_tau, failure_criterion_state, acumulated_damage, time_steps);
            }

            if (this->Is(DEMFlags::HAS_STRESS_TENSOR) /*&& (i < mContinuumInitialNeighborsSize)*/) {
                AddNeighbourContributionToStressTensor(TotalGlobalElasticContactForce, data_buffer.mLocalCoordSystem[2], data_buffer.mDistance, radius_sum, this);
            }

            AddContributionToRepresentativeVolume(data_buffer.mDistance, radius_sum, calculation_area);

            ComputeForceWithNeighbourFinalOperations();
        } // for each neighbor

        ComputeBrokenBondsRatio();
    } //  ComputeBallToBallContactForce

    void BeamParticle::Calculate(const Variable<double>& rVariable, double& Output, const ProcessInfo& r_process_info)
    {
        KRATOS_TRY

        //CRITICAL DELTA CALCULATION

        if (rVariable == DELTA_TIME) {
            double mass = GetMass();
            double coeff = r_process_info[NODAL_MASS_COEFF];

            if (coeff > 1.0) {
                KRATOS_THROW_ERROR(std::runtime_error, "The coefficient assigned for virtual mass is larger than one. Virtual_mass_coeff is ", coeff);
            }

            else if ((coeff == 1.0) && (r_process_info[VIRTUAL_MASS_OPTION])) {
                Output = 9.0E09;
            }

            else {

                if (r_process_info[VIRTUAL_MASS_OPTION]) {
                    mass = mass / (1 - coeff);
                }

                double eq_mass = 0.5 * mass; //"mass" of the contact

                double kn = 0.0;
                double kt = 0.0;

                double ini_delta = 0.05 * GetInteractionRadius(); // Hertz needs an initial Delta, linear ignores it

                mDiscontinuumConstitutiveLaw->GetContactStiffness(this, this, ini_delta, kn, kt);

                //double K = Globals::Pi * GetYoung() * GetRadius(); //M. Error, should be the same that the local definition.

                Output = 0.34 * sqrt(eq_mass / kn);

                if (this->Is(DEMFlags::HAS_ROTATION)) {
                    //Output *= 0.5; //factor for critical time step when rotation is allowed.
                }
            }

            return;
        }

        if (rVariable == PARTICLE_TRANSLATIONAL_KINEMATIC_ENERGY) {

          const array_1d<double, 3>& vel = this->GetGeometry()[0].FastGetSolutionStepValue(VELOCITY);
          double square_of_celerity      = vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2];

          Output = 0.5 * (GetMass() * square_of_celerity);

          return;
        }

        if (rVariable == PARTICLE_ROTATIONAL_KINEMATIC_ENERGY) {

            const array_1d<double, 3> ang_vel = this->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY);
            array_1d<double, 3> base_principal_moments_of_inertia = GetGeometry()[0].GetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA);
            Output = 0.5 * (base_principal_moments_of_inertia[0] * ang_vel[0] * ang_vel[0] +
                            base_principal_moments_of_inertia[1] * ang_vel[1] * ang_vel[1] +
                            base_principal_moments_of_inertia[2] * ang_vel[2] * ang_vel[2]);

            return;
        }

        if (rVariable == PARTICLE_ELASTIC_ENERGY) {

            Output = GetElasticEnergy();

        }

        if (rVariable == PARTICLE_INELASTIC_FRICTIONAL_ENERGY) {

            Output = GetInelasticFrictionalEnergy();

        }

        if (rVariable == PARTICLE_INELASTIC_VISCODAMPING_ENERGY) {

            Output = GetInelasticViscodampingEnergy();

        }

        AdditionalCalculate(rVariable, Output, r_process_info);

        KRATOS_CATCH("")

    } //Calculate

    void BeamParticle::Move(const double delta_t, const bool rotation_option, const double force_reduction_factor, const int StepFlag) {
        GetTranslationalIntegrationScheme().Move(GetGeometry()[0], delta_t, force_reduction_factor, StepFlag);
        if (rotation_option) {
            GetRotationalIntegrationScheme().RotateBeam(GetGeometry()[0], delta_t, force_reduction_factor, StepFlag);
        }
    }

    void BeamParticle::AddContributionToRepresentativeVolume(const double distance, const double radius_sum, const double contact_area) {}

    void BeamParticle::FinalizeSolutionStep(ProcessInfo& r_process_info) {

        ComputeReactions();

        // this->GetGeometry()[0].FastGetSolutionStepValue(REPRESENTATIVE_VOLUME) = mPartialRepresentativeVolume;
        double& rRepresentative_Volume = this->GetGeometry()[0].FastGetSolutionStepValue(REPRESENTATIVE_VOLUME);

        if (this->Is(DEMFlags::HAS_STRESS_TENSOR)) {

            //Divide Stress Tensor by the total volume:
            //const array_1d<double, 3>& reaction_force=this->GetGeometry()[0].FastGetSolutionStepValue(FORCE_REACTION);

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    (*mStressTensor)(i,j) /= rRepresentative_Volume;
                }
            }

            SymmetrizeStressTensor();
        }

        //Update sphere mass and inertia taking into account the real volume of the represented volume:
        // SetMass(this->GetGeometry()[0].FastGetSolutionStepValue(REPRESENTATIVE_VOLUME) * GetDensity());
    }

    double BeamParticle::GetParticleInitialCohesion()            { return SphericParticle::GetFastProperties()->GetParticleInitialCohesion();            }
    double BeamParticle::GetAmountOfCohesionFromStress()         { return SphericParticle::GetFastProperties()->GetAmountOfCohesionFromStress();         }
    double BeamParticle::GetParticleConicalDamageContactRadius() { return SphericParticle::GetFastProperties()->GetParticleConicalDamageContactRadius(); }
    double BeamParticle::GetParticleConicalDamageMaxStress()     { return SphericParticle::GetFastProperties()->GetParticleConicalDamageMaxStress();     }
    double BeamParticle::GetParticleConicalDamageGamma()         { return SphericParticle::GetFastProperties()->GetParticleConicalDamageGamma();         }
    double BeamParticle::GetLevelOfFouling()                     { return SphericParticle::GetFastProperties()->GetLevelOfFouling();                     }

    void   BeamParticle::SetParticleInitialCohesionFromProperties(double* particle_initial_cohesion)          { SphericParticle::GetFastProperties()->SetParticleInitialCohesionFromProperties( particle_initial_cohesion);  }
    void   BeamParticle::SetAmountOfCohesionFromStressFromProperties(double* amount_of_cohesion_from_stress)  { SphericParticle::GetFastProperties()->SetAmountOfCohesionFromStressFromProperties( amount_of_cohesion_from_stress);  }
    void   BeamParticle::SetParticleConicalDamageContactRadiusFromProperties(double* particle_contact_radius) { SphericParticle::GetFastProperties()->SetParticleConicalDamageContactRadiusFromProperties( particle_contact_radius); }
    void   BeamParticle::SetParticleConicalDamageMaxStressFromProperties(double* particle_max_stress)         { SphericParticle::GetFastProperties()->SetParticleConicalDamageMaxStressFromProperties( particle_max_stress);         }
    void   BeamParticle::SetParticleConicalDamageGammaFromProperties(double* particle_gamma)                  { SphericParticle::GetFastProperties()->SetParticleConicalDamageGammaFromProperties( particle_gamma);                  }
    void   BeamParticle::SetLevelOfFoulingFromProperties(double* level_of_fouling)                            { SphericParticle::GetFastProperties()->SetLevelOfFoulingFromProperties( level_of_fouling);                            }

    double BeamParticle::SlowGetParticleInitialCohesion()            { return GetProperties()[PARTICLE_INITIAL_COHESION]; }
    double BeamParticle::SlowGetAmountOfCohesionFromStress()         { return GetProperties()[AMOUNT_OF_COHESION_FROM_STRESS]; }
    double BeamParticle::SlowGetParticleConicalDamageContactRadius() { return GetProperties()[CONICAL_DAMAGE_CONTACT_RADIUS];  }
    double BeamParticle::SlowGetParticleConicalDamageMaxStress()     { return GetProperties()[CONICAL_DAMAGE_MAX_STRESS];      }
    double BeamParticle::SlowGetParticleConicalDamageGamma()         { return GetProperties()[CONICAL_DAMAGE_GAMMA];           }
    double BeamParticle::SlowGetLevelOfFouling()                     { return GetProperties()[LEVEL_OF_FOULING];               }

} // namespace Kratos
