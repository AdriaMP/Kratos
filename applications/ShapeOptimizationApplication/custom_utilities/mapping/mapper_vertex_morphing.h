// ==============================================================================
//  KratosShapeOptimizationApplication
//
//  License:         BSD License
//                   license: ShapeOptimizationApplication/license.txt
//
//  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
//
// ==============================================================================

#ifndef MAPPER_VERTEX_MORPHING_H
#define MAPPER_VERTEX_MORPHING_H

// ------------------------------------------------------------------------------
// System includes
// ------------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <algorithm>

// ------------------------------------------------------------------------------
// Project includes
// ------------------------------------------------------------------------------
#include "includes/define.h"
#include "includes/model_part.h"
#include "spatial_containers/spatial_containers.h"
#include "utilities/builtin_timer.h"
#include "spaces/ublas_space.h"
#include "mapper_base.h"
#include "filter_function.h"

// ==============================================================================

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/

class MapperVertexMorphing : public Mapper
{
public:
    ///@name Type Definitions
    ///@{

    // Type definitions for better reading later
    typedef Node < 3 > NodeType;
    typedef Node < 3 > ::Pointer NodeTypePointer;
    typedef std::vector<NodeType::Pointer> NodeVector;
    typedef std::vector<NodeType::Pointer>::iterator NodeIterator;
    typedef std::vector<double>::iterator DoubleVectorIterator;
    typedef ModelPart::ConditionsContainerType ConditionsArrayType;
    typedef array_1d<double,3> array_3d;

    // Type definitions for linear algebra including sparse systems
    typedef UblasSpace<double, CompressedMatrix, Vector> SparseSpaceType;
    typedef SparseSpaceType::MatrixType SparseMatrixType;
    typedef SparseSpaceType::VectorType VectorType;

    // Type definitions for tree-search
    typedef Bucket< 3, NodeType, NodeVector, NodeTypePointer, NodeIterator, DoubleVectorIterator > BucketType;
    typedef Tree< KDTreePartition<BucketType> > KDTree;

    /// Pointer definition of MapperVertexMorphing
    KRATOS_CLASS_POINTER_DEFINITION(MapperVertexMorphing);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    MapperVertexMorphing( ModelPart& rOriginiMdpa, ModelPart& rDestinationMdpa, Parameters MapperSettings )
        : mrOriginMdpa(rOriginiMdpa),
          mrDestinationMdpa(rDestinationMdpa),
          mOriginNodeNumber(rOriginiMdpa.Nodes().size()),
          mDestinationNodeNumber(rDestinationMdpa.Nodes().size()),
          mMapperSettings(MapperSettings)
    {
    }

    /// Destructor.
    virtual ~MapperVertexMorphing()
    {
    }

    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    // --------------------------------------------------------------------------
    void Initialize() override
    {
        BuiltinTimer timer;
        std::cout << "> Starting initialization of mapping..." << std::endl;

        if (mIsMappingInitialized == false)
        {
            CreateListOfNodesInOriginMdpa();
            CreateFilterFunction();
            InitializeMappingVariables();
            AssignMappingIds();
        }

        InitializeComputationOfMappingMatrix();
        CreateSearchTreeWithAllNodesInOriginMdpa();
        ComputeMappingMatrix();

        mIsMappingInitialized = true;

        std::cout << "> Finished initialization of mapping in " << timer.ElapsedSeconds() << " s." << std::endl;
    }

    // --------------------------------------------------------------------------
    void Map( const Variable<array_3d> &rVariable, const Variable<array_3d> &rMappedVariable) override
    {
        if (mIsMappingInitialized == false)
            Initialize();

        BuiltinTimer mapping_time;
        std::cout << "\n> Starting mapping of " << rVariable.Name() << "..." << std::endl;

        // Prepare vectors for mapping
        mXValuesOrigin.clear();
        mYValuesOrigin.clear();
        mZValuesOrigin.clear();
        mXValuesDestination.clear();
        mYValuesDestination.clear();
        mZValuesDestination.clear();

        for(auto& node_i : mrOriginMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            array_3d& nodal_variable = node_i.FastGetSolutionStepValue(rVariable);
            mXValuesOrigin[i] = nodal_variable[0];
            mYValuesOrigin[i] = nodal_variable[1];
            mZValuesOrigin[i] = nodal_variable[2];
        }

        // Perform mapping
        noalias(mXValuesDestination) = prod(mMappingMatrix,mXValuesOrigin);
        noalias(mYValuesDestination) = prod(mMappingMatrix,mYValuesOrigin);
        noalias(mZValuesDestination) = prod(mMappingMatrix,mZValuesOrigin);

        // Assign results to nodal variable
        for(auto& node_i : mrDestinationMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);

            Vector node_vector = ZeroVector(3);
            node_vector(0) = mXValuesDestination[i];
            node_vector(1) = mYValuesDestination[i];
            node_vector(2) = mZValuesDestination[i];
            node_i.FastGetSolutionStepValue(rMappedVariable) = node_vector;
        }

        std::cout << "> Finished mapping in " << mapping_time.ElapsedSeconds() << " s." << std::endl;
    }

    // --------------------------------------------------------------------------
    void Map( const Variable<double> &rVariable, const Variable<double> &rMappedVariable) override
    {
        if (mIsMappingInitialized == false)
            Initialize();

        BuiltinTimer mapping_time;
        std::cout << "\n> Starting mapping of " << rVariable.Name() << "..." << std::endl;

        // Prepare vectors for mapping
        mXValuesOrigin.clear();
        mXValuesDestination.clear();

        for(auto& node_i : mrOriginMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            mXValuesOrigin[i] = node_i.FastGetSolutionStepValue(rVariable);
        }

        // Perform mapping
        noalias(mXValuesDestination) = prod(mMappingMatrix,mXValuesOrigin);

        // Assign results to nodal variable
        for(auto& node_i : mrDestinationMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            node_i.FastGetSolutionStepValue(rMappedVariable) = mXValuesDestination[i];
        }

        std::cout << "> Finished mapping in " << mapping_time.ElapsedSeconds() << " s." << std::endl;
    }

    // --------------------------------------------------------------------------
    void InverseMap( const Variable<array_3d> &rVariable, const Variable<array_3d> &rMappedVariable) override
    {
        if (mIsMappingInitialized == false)
            Initialize();

        BuiltinTimer mapping_time;
        std::cout << "\n> Starting inverse mapping of " << rVariable.Name() << "..." << std::endl;

        // Prepare vectors for mapping
        mXValuesOrigin.clear();
        mYValuesOrigin.clear();
        mZValuesOrigin.clear();
        mXValuesDestination.clear();
        mYValuesDestination.clear();
        mZValuesDestination.clear();

        for(auto& node_i : mrDestinationMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            array_3d& nodal_variable = node_i.FastGetSolutionStepValue(rVariable);
            mXValuesDestination[i] = nodal_variable[0];
            mYValuesDestination[i] = nodal_variable[1];
            mZValuesDestination[i] = nodal_variable[2];
        }

        // Perform mapping
        if(mMapperSettings["apply_consistent_mapping"].GetBool())
        {
            KRATOS_ERROR_IF(mOriginNodeNumber != mDestinationNodeNumber) << "Consisten mapping requires matching origin and destination model part.";

            noalias(mXValuesOrigin) = prod(mMappingMatrix,mXValuesDestination);
            noalias(mYValuesOrigin) = prod(mMappingMatrix,mYValuesDestination);
            noalias(mZValuesOrigin) = prod(mMappingMatrix,mZValuesDestination);
        }
        else
        {
            SparseSpaceType::TransposeMult(mMappingMatrix,mXValuesDestination,mXValuesOrigin);
            SparseSpaceType::TransposeMult(mMappingMatrix,mYValuesDestination,mYValuesOrigin);
            SparseSpaceType::TransposeMult(mMappingMatrix,mZValuesDestination,mZValuesOrigin);
        }

        // Assign results to nodal variable
        for(auto& node_i : mrOriginMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);

            Vector node_vector = ZeroVector(3);
            node_vector(0) = mXValuesOrigin[i];
            node_vector(1) = mYValuesOrigin[i];
            node_vector(2) = mZValuesOrigin[i];
            node_i.FastGetSolutionStepValue(rMappedVariable) = node_vector;
        }

        std::cout << "> Finished mapping in " << mapping_time.ElapsedSeconds() << " s." << std::endl;
    }

    // --------------------------------------------------------------------------
    void InverseMap(const Variable<double> &rVariable, const Variable<double> &rMappedVariable) override
    {
        if (mIsMappingInitialized == false)
            Initialize();

        BuiltinTimer mapping_time;
        std::cout << "\n> Starting inverse mapping of " << rVariable.Name() << "..." << std::endl;

        // Prepare vectors for mapping
        mXValuesOrigin.clear();
        mXValuesDestination.clear();

        for(auto& node_i : mrDestinationMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            mXValuesDestination[i] = node_i.FastGetSolutionStepValue(rVariable);
        }

        // Perform mapping
        if(mMapperSettings["apply_consistent_mapping"].GetBool())
        {
            KRATOS_ERROR_IF(mOriginNodeNumber != mDestinationNodeNumber) << "Consisten mapping requires matching origin and destination model part.";
            noalias(mXValuesOrigin) = prod(mMappingMatrix,mXValuesDestination);
        }
        else
            SparseSpaceType::TransposeMult(mMappingMatrix,mXValuesDestination,mXValuesOrigin);

        // Assign results to nodal variable
        for(auto& node_i : mrOriginMdpa.Nodes())
        {
            int i = node_i.GetValue(MAPPING_ID);
            node_i.FastGetSolutionStepValue(rMappedVariable) = mXValuesOrigin[i];
        }

        std::cout << "> Finished mapping in " << mapping_time.ElapsedSeconds() << " s." << std::endl;
    }

    // --------------------------------------------------------------------------

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        return "MapperVertexMorphing";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << "MapperVertexMorphing";
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const
    {
    }


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{

    // Initialized by class constructor
    ModelPart& mrOriginMdpa;
    ModelPart& mrDestinationMdpa;
    FilterFunction::Pointer mpFilterFunction;
    Parameters mMapperSettings;
    bool mIsMappingInitialized = false;

    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{

    virtual void InitializeComputationOfMappingMatrix()
    {
        mpSearchTree.reset();
        mMappingMatrix.clear();
    }

    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    // Initialized by class constructor
    const unsigned int mOriginNodeNumber;
    const unsigned int mDestinationNodeNumber;

    // Variables for spatial search
    unsigned int mBucketSize = 100;
    NodeVector mListOfNodesInOriginMdpa;
    KDTree::Pointer mpSearchTree;

    // Variables for mapping
    SparseMatrixType mMappingMatrix;
    Vector mXValuesOrigin, mYValuesOrigin, mZValuesOrigin;
    Vector mXValuesDestination, mYValuesDestination, mZValuesDestination;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{

    // --------------------------------------------------------------------------
    void CreateListOfNodesInOriginMdpa()
    {
        mListOfNodesInOriginMdpa.resize(mOriginNodeNumber);
        int counter = 0;
        for (ModelPart::NodesContainerType::iterator node_it = mrOriginMdpa.NodesBegin(); node_it != mrOriginMdpa.NodesEnd(); ++node_it)
        {
            NodeTypePointer pnode = *(node_it.base());
            mListOfNodesInOriginMdpa[counter++] = pnode;
        }
    }

    // --------------------------------------------------------------------------
    void CreateFilterFunction()
    {
        std::string filter_type = mMapperSettings["filter_function_type"].GetString();
        double filter_radius = mMapperSettings["filter_radius"].GetDouble();

        mpFilterFunction = Kratos::shared_ptr<FilterFunction>(new FilterFunction(filter_type, filter_radius));
    }

    // --------------------------------------------------------------------------
    void InitializeMappingVariables()
    {
        mMappingMatrix.resize(mDestinationNodeNumber,mOriginNodeNumber,false);
        mMappingMatrix.clear();

        mXValuesOrigin.resize(mOriginNodeNumber,0.0);
        mYValuesOrigin.resize(mOriginNodeNumber,0.0);
        mZValuesOrigin.resize(mOriginNodeNumber,0.0);

        mXValuesDestination.resize(mDestinationNodeNumber,0.0);
        mYValuesDestination.resize(mDestinationNodeNumber,0.0);
        mZValuesDestination.resize(mDestinationNodeNumber,0.0);
    }

    // --------------------------------------------------------------------------
    void AssignMappingIds()
    {
        unsigned int i = 0;
        for(auto& node_i : mrOriginMdpa.Nodes())
            node_i.SetValue(MAPPING_ID,i++);

        i = 0;
        for(auto& node_i : mrDestinationMdpa.Nodes())
            node_i.SetValue(MAPPING_ID,i++);
    }

    // --------------------------------------------------------------------------
    void CreateSearchTreeWithAllNodesInOriginMdpa()
    {
        mpSearchTree = Kratos::shared_ptr<KDTree>(new KDTree(mListOfNodesInOriginMdpa.begin(), mListOfNodesInOriginMdpa.end(), mBucketSize));
    }

    // --------------------------------------------------------------------------
    void ComputeMappingMatrix()
    {
        double filter_radius = mMapperSettings["filter_radius"].GetDouble();
        unsigned int max_number_of_neighbors = mMapperSettings["max_nodes_in_filter_radius"].GetInt();

        for(auto& node_i : mrDestinationMdpa.Nodes())
        {
            NodeVector neighbor_nodes( max_number_of_neighbors );
            std::vector<double> resulting_squared_distances( max_number_of_neighbors );
            unsigned int number_of_neighbors = mpSearchTree->SearchInRadius( node_i,
                                                                             filter_radius,
                                                                             neighbor_nodes.begin(),
                                                                             resulting_squared_distances.begin(),
                                                                             max_number_of_neighbors );



            std::vector<double> list_of_weights( number_of_neighbors, 0.0 );
            double sum_of_weights = 0.0;

            if(number_of_neighbors >= max_number_of_neighbors)
                std::cout << "\n> WARNING!!!!! For node " << node_i.Id() << " and specified filter radius, maximum number of neighbor nodes (=" << max_number_of_neighbors << " nodes) reached!" << std::endl;

            ComputeWeightForAllNeighbors( node_i, neighbor_nodes, number_of_neighbors, list_of_weights, sum_of_weights );
            FillMappingMatrixWithWeights( node_i, neighbor_nodes, number_of_neighbors, list_of_weights, sum_of_weights );
        }
    }

    // --------------------------------------------------------------------------
    virtual void ComputeWeightForAllNeighbors(  ModelPart::NodeType& origin_node,
                                        NodeVector& neighbor_nodes,
                                        unsigned int number_of_neighbors,
                                        std::vector<double>& list_of_weights,
                                        double& sum_of_weights )
    {
        for(unsigned int neighbor_itr = 0 ; neighbor_itr<number_of_neighbors ; neighbor_itr++)
        {
            ModelPart::NodeType& neighbor_node = *neighbor_nodes[neighbor_itr];
            double weight = mpFilterFunction->compute_weight( origin_node.Coordinates(), neighbor_node.Coordinates() );

            list_of_weights[neighbor_itr] = weight;
            sum_of_weights += weight;
        }
    }

    // --------------------------------------------------------------------------
    void FillMappingMatrixWithWeights(  ModelPart::NodeType& origin_node,
                                        NodeVector& neighbor_nodes,
                                        unsigned int number_of_neighbors,
                                        std::vector<double>& list_of_weights,
                                        double& sum_of_weights )
    {


        unsigned int row_id = origin_node.GetValue(MAPPING_ID);
        for(unsigned int neighbor_itr = 0 ; neighbor_itr<number_of_neighbors ; neighbor_itr++)
        {
            ModelPart::NodeType& neighbor_node = *neighbor_nodes[neighbor_itr];
            int collumn_id = neighbor_node.GetValue(MAPPING_ID);


            double weight = list_of_weights[neighbor_itr] / sum_of_weights;
            mMappingMatrix.insert_element(row_id,collumn_id,weight);
        }
    }

    // --------------------------------------------------------------------------

    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
//      MapperVertexMorphing& operator=(MapperVertexMorphing const& rOther);

    /// Copy constructor.
//      MapperVertexMorphing(MapperVertexMorphing const& rOther);


    ///@}

}; // Class MapperVertexMorphing

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

///@}


}  // namespace Kratos.

#endif // MAPPER_VERTEX_MORPHING_H
