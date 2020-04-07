//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//

#if !defined(KRATOS_SPARSE_CONTIGUOUS_ROW_GRAPH_H_INCLUDED )
#define  KRATOS_SPARSE_CONTIGUOUS_ROW_GRAPH_H_INCLUDED


// System includes
#include <iostream>
#include "includes/ublas_interface.h"
#include "includes/serializer.h"

// External includes
#include <unordered_map>
#include <unordered_set>

// Project includes
#include "includes/define.h"


namespace Kratos
{
///@addtogroup KratosCore
///@{

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
//class to construct and store a matrix graph. Can be used to construct efficiently a CSR matrix (or other sparse matrix types)

/** This class is designed to store a matrix graph, aimed at the fast construction of other
 * sparse matrix formats (particularly CSR)
 * IMPORTANT NOTE: it is BY DESIGN NOT threadsafe! (a graph should be computed in each thread and then merged)
*/

class SparseContiguousRowGraph
{
public:
    ///@name Type Definitions
    ///@{
    typedef std::size_t IndexType;
    typedef DenseVector<std::unordered_set<IndexType> > GraphType; //using a map since we need it ordered
    typedef typename GraphType::const_iterator const_row_iterator;

    /// Pointer definition of SparseContiguousRowGraph
    KRATOS_CLASS_POINTER_DEFINITION(SparseContiguousRowGraph);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    SparseContiguousRowGraph(IndexType GraphSize)
    :mGraphSize(GraphSize)
    {
        mGraph.resize(GraphSize);
    }

    /// Destructor.
    virtual ~SparseContiguousRowGraph(){}

    /// Assignment operator. TODO: decide if we do want to allow it
    SparseContiguousRowGraph& operator=(SparseContiguousRowGraph const& rOther)=delete;
    // {
    //     this->AddEntries(rOther.GetGraph());
    //     return *this;
    // }

    /// Copy constructor. TODO: we need it otherwise the sendrecv does not work...
    ///but i don't know why :-(
    SparseContiguousRowGraph(const SparseContiguousRowGraph& rOther)
    {
        this->AddEntries(rOther);
    }

    ///@}
    ///@name Operators
    ///@{
    void Clear()
    {
        mGraph.clear();
    }

    inline IndexType Size() const{
        return mGraphSize;
    }

    bool Has(const IndexType I, const IndexType J) const
    {
        const auto& row_it = mGraph[I];
        if((row_it->second).find(J) != (row_it->second).end())
            return true;
        return false;
    }

    inline IndexType LocalIndex(const IndexType GlobalIndex) const{
        return GlobalIndex;
    }

    inline IndexType GlobalIndex(const IndexType LocalIndex) const{
         return LocalIndex;
    }

    void AddEntry(const IndexType RowIndex, const IndexType ColIndex)
    {
        mGraph[RowIndex].insert(ColIndex);
    }

    template<class TContainerType>
    void AddEntries(const IndexType RowIndex, const TContainerType& rColIndices)
    {
        mGraph[RowIndex].insert(rColIndices.begin(), rColIndices.end());
    }

    template<class TIteratorType>
    void AddEntries(const IndexType RowIndex,
                    const TIteratorType& rColBegin,
                    const TIteratorType& rColEnd
                    )
    {
        mGraph[RowIndex].insert(rColBegin, rColEnd);
    }

    //adds a square FEM matrix, identified by rIndices
    template<class TContainerType>
    void AddEntries(const TContainerType& rIndices)
    {
        for(auto I : rIndices){
            KRATOS_DEBUG_ERROR_IF(I > this->Size()) << "Index : " << I
                << " exceeds the graph size : " << Size() << std::endl;
            mGraph[I].insert(rIndices.begin(), rIndices.end());
        }
    }

    void AddEntries(const SparseContiguousRowGraph& rOtherGraph)
    {
        for(IndexType i=0; i<rOtherGraph.Size(); ++i)
        {
            AddEntries(i, rOtherGraph.GetGraph()[i]);
        }
    }

    void Finalize()
    {

    }

    const GraphType& GetGraph() const{
        return mGraph;
    }

    // IndexType ExportCSRArrays(
    //     vector<IndexType>& rRowIndices,
    //     vector<IndexType>& rColIndices
    // ) //TODO: this function should be imported in the CSR matrix interface, not here
    // {
    //     //need to detect the number of rows this way since there may be gaps
    //     IndexType nrows=0;
    //     for(const auto& item : this->GetGraph())
    //     {
    //         nrows = std::max(nrows,item.first+1);
    //     }

    //     if(rRowIndices.size() != nrows+1)
    //     {
    //         rRowIndices.resize(nrows+1, false);
    //     }
    //     //set it to zero in parallel to allow first touching
    //     #pragma omp parallel for
    //     for(int i=0; i<static_cast<int>(nrows+1); ++i)
    //         rRowIndices[i] = 0;

    //     //count the entries TODO: do the loop in parallel if possible
    //     for(const auto& item : this->GetGraph())
    //     {
    //         rRowIndices[item.first+1] = item.second.size();
    //     }
    //     //sum entries
    //     for(int i = 1; i<static_cast<int>(rRowIndices.size()); ++i){
    //         rRowIndices[i] += rRowIndices[i-1];
    //     }


    //     IndexType nnz = rRowIndices[nrows];
    //     if(rColIndices.size() != nnz){
    //         rColIndices.resize(nnz, false);
    //     }
    //     //set it to zero in parallel to allow first touching
    //     #pragma omp parallel for
    //     for(int i=0; i<static_cast<int>(rColIndices.size());++i)
    //         rColIndices[i] = 0;

    //     //count the entries TODO: do the loop in parallel if possible
    //     for(const auto& item : this->GetGraph()){
    //         IndexType start = rRowIndices[item.first];

    //         IndexType counter = 0;
    //         for(auto index : item.second){
    //             rColIndices[start+counter] = index;
    //             counter++;
    //         }
    //     }

    //     //reorder columns
    //     #pragma omp parallel for
    //     for(int i=0; i<static_cast<int>(rRowIndices.size()-1);++i){
    //         std::sort(rColIndices.begin()+rRowIndices[i], rColIndices.begin()+rRowIndices[i+1]);
    //     }
    //     return nrows;
    // }

    ///@}
    ///@name Operations
    ///@{


    ///@}
    ///@name Access
    ///@{
    class const_iterator_adaptor : public std::iterator<
        std::forward_iterator_tag,
        typename GraphType::value_type
        >
	{
		const_row_iterator map_iterator;
        const_row_iterator mbegin;
	public:
		const_iterator_adaptor(const_row_iterator it) :map_iterator(it),mbegin(it) {}
		const_iterator_adaptor(const const_iterator_adaptor& it)
            : map_iterator(it.map_iterator),mbegin(it.mbegin) {}
		const_iterator_adaptor& operator++() { map_iterator++; return *this; }
		const_iterator_adaptor operator++(int) { const_iterator_adaptor tmp(*this); operator++(); return tmp; }
		bool operator==(const const_iterator_adaptor& rhs) const
            { return map_iterator == rhs.map_iterator; }
		bool operator!=(const const_iterator_adaptor& rhs) const
            { return map_iterator != rhs.map_iterator; }
        //TODO: is it correct that the two following operators are the same?
		const typename GraphType::value_type& operator*() const { return *map_iterator; }
		const typename GraphType::value_type& operator->() const { return *map_iterator; }
		const_row_iterator& base() { return map_iterator; }
		const_row_iterator const& base() const { return map_iterator; }
        const IndexType GetRowIndex(){
            return map_iterator-mbegin;
        }
	};

    const_iterator_adaptor begin() const
    {
        return const_iterator_adaptor( mGraph.begin() );
    }
    const_iterator_adaptor end() const
    {
        return const_iterator_adaptor( mGraph.end() );
    }


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        std::stringstream buffer;
        buffer << "SparseContiguousRowGraph" ;
        return buffer.str();
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const {rOStream << "SparseContiguousRowGraph";}

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const {}

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


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


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
    IndexType mGraphSize;
    GraphType mGraph;

    ///@}
    ///@name Private Operators
    ///@{
    friend class Serializer;

    void save(Serializer& rSerializer) const
    {
        const IndexType N = this->Size();
        rSerializer.save("GraphSize",N);
        for(IndexType I=0; I<N; ++I)
        {
//            rSerializer.save("I",I);

            unsigned int Nr = mGraph[I].size();
            rSerializer.save("Nr",Nr);
            for(auto J : mGraph[I]){
                rSerializer.save("J",J);
            }
        }
    }

    void load(Serializer& rSerializer)
    {
        rSerializer.load("GraphSize",mGraphSize);

        for(IndexType I=0; I<mGraphSize; ++I)
        {
//            IndexType I;
//            rSerializer.load("I",I);

            unsigned int Nr;
            rSerializer.load("Nr",Nr);
            for(unsigned int k=0; k<Nr; ++k){
                IndexType J;
                rSerializer.load("J",J);
                AddEntry(I,J);
            }
        }
    }


    ///@}
    ///@name Private Operations
    ///@{
    SparseContiguousRowGraph(){};


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{



    ///@}

}; // Class SparseContiguousRowGraph

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                SparseContiguousRowGraph& rThis){
                    return rIStream;
                }

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                const SparseContiguousRowGraph& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_SPARSE_CONTIGUOUS_ROW_GRAPH_H_INCLUDED  defined


