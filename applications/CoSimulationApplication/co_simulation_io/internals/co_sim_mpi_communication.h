// KRATOS  / ___|___/ ___|(_)_ __ ___  _   _| | __ _| |_(_) ___  _ ___
//        | |   / _ \___ \| | '_ ` _ \| | | | |/ _` | __| |/ _ \| '_  |
//        | |__| (_) |__) | | | | | | | |_| | | (_| | |_| | (_) | | | |
//         \____\___/____/|_|_| |_| |_|\__,_|_|\__,_|\__|_|\___/|_| |_|
//
//  License:		 BSD License
//                   license: CoSimulationApplication/license.txt
//
//  Main authors:    Philipp Bucher
//

#ifndef KRATOS_CO_SIM_MPI_COMM_H_INCLUDED
#define KRATOS_CO_SIM_MPI_COMM_H_INCLUDED

// System includes
#include "mpi.h"

// Project includes
#include "co_sim_communication.h"

namespace CoSimIO {

class CoSimMPICommunication : public CoSimCommunication
{
public:
    explicit CoSimMPICommunication(const std::string& rName, SettingsType& rSettings, const bool IsConnectionMaster)
        : CoSimCommunication(rName, rSettings, IsConnectionMaster)
    {
       KRATOS_CO_SIM_ERROR << "MPI Communication is not implemented yet" << std::endl;
    }

};

} // namespace CoSimIO

#endif /* KRATOS_CO_SIM_MPI_COMM_H_INCLUDED */