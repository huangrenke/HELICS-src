/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/chelics.h"

#define CE(status) BOOST_CHECK_EQUAL(status, helics_ok)
#define HELICS_SIZE_MAX 512

#ifdef QUICK_TESTS_ONLY
#ifndef DISABLE_TCP_CORE
const std::string core_types[] = {"test", "ipc_2", "tcp", "test_2", "zmq", "udp","test_3","zmq_3" };
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp","test_3","zmq_3"};
#else
const std::string core_types[] = {"test", "ipc_2", "test_2", "zmq", "udp","test_3","zmq_3" };
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp","test_3","zmq_3" };
#endif
#else
#ifndef DISABLE_TCP_CORE
const std::string core_types[] = {"test",   "ipc",   "zmq",   "udp",   "tcp",
                                  "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2",
                                "test_3", "zmq_3", "udp_3", "tcp_3" ,
                                "test_4", "zmq_4", "udp_4", "tcp_4" };
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp",
                                        "test_3", "zmq_3", "udp_3", "tcp_3" };
#else
const std::string core_types[] = {"test", "ipc", "zmq", "udp", "test_2", "ipc_2", "zmq_2", "udp_2", "test_3", "zmq_3", "udp_3" ,
"test_4", "zmq_4", "udp_4" };
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp","test_3", "zmq_3", "udp_3" };
#endif
#endif

#ifndef DISABLE_TCP_CORE
const std::string travis_core_types[] = { "test",   "ipc",   "tcp",   "zmq",   "udp",
"test_2","zmq_4", "ipc_2","test_4", "tcp_2", "zmq_2", "udp_2","test_3","zmq_3" };
#else
const std::string travis_core_types[] = { "test", "ipc", "zmq", "udp", "test_2","zmq_4", "ipc_2","test_4", "zmq_2", "udp_2","test_3","zmq_3" };
#endif

typedef helics_federate (*FedCreator)(helics_federate_info_t);

struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture ();

    std::shared_ptr<helics_broker> AddBroker (const std::string &core_type_name, int count);
    std::shared_ptr<helics_broker>
    AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    void SetupTest (
            FedCreator ctor,
            std::string core_type_name,
            int count,
            helics_time_t time_delta = helics_time_zero,
            std::string name_prefix = "fed")
    {
        std::shared_ptr<helics_broker> broker = AddBroker (core_type_name, count);
        AddFederates (ctor, core_type_name, count, broker, time_delta, name_prefix);
    }

    std::vector<std::shared_ptr<helics_federate>> AddFederates (
            FedCreator ctor,
            std::string core_type_name,
            int count,
            std::shared_ptr<helics_broker> broker,
            helics_time_t time_delta = helics_time_zero,
            std::string name_prefix = "fed")
    {
        char tmp[HELICS_SIZE_MAX];
        bool hasIndex = hasIndexCode (core_type_name);
        int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back ();
            core_type_name.pop_back ();
        }

        std::string initString = std::string ("--broker=");
        CE (helicsBrokerGetIdentifier (broker.get(), tmp, HELICS_SIZE_MAX));
        initString += tmp;
        initString += " --broker_address=";
        CE (helicsBrokerGetAddress (broker.get(), tmp, HELICS_SIZE_MAX));
        initString += tmp;

        if (!extraCoreArgs.empty ())
        {
            initString.push_back (' ');
            initString.append (extraCoreArgs);
        }

        helics_federate_info_t fi = helicsFederateInfoCreate ();
        CE( helicsFederateInfoSetFederateName (fi, ""));
        CE( helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str()));
        CE( helicsFederateInfoSetTimeDelta (fi, time_delta));

        std::vector<std::shared_ptr<helics_federate>> federates_added;

        

        switch (setup)
        {
        case 1:
        default:
        {
            auto init = initString + " --federates "+ std::to_string(count);
            auto core = helicsCreateCore (core_type_name.c_str(), NULL, init.c_str());
            CE (helicsCoreGetIdentifier (core, tmp, HELICS_SIZE_MAX));
            CE (helicsFederateInfoSetCoreName (fi, tmp));
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto name = name_prefix + std::to_string (ii+offset);
                CE (helicsFederateInfoSetFederateName (fi, name.c_str()));
                auto fed = std::make_shared<helics_federate> (ctor (fi));
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 2:
        {  // each federate has its own core
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto init = initString + " --federates 1";
                auto core = helicsCreateCore (core_type_name.c_str(), NULL, init.c_str());
                CE (helicsCoreGetIdentifier (core, tmp, HELICS_SIZE_MAX));
                CE (helicsFederateInfoSetCoreName (fi, tmp));

                auto name = name_prefix + std::to_string (ii+offset);
                CE (helicsFederateInfoSetFederateName (fi, name.c_str()));
                auto fed = std::make_shared<helics_federate> (ctor (fi));
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 3:
        {
            auto subbroker = AddBroker(core_type_name, initString + " --federates " + std::to_string(count));
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii)
            {
                AddFederates (ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 4:
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii)
            {
                auto subbroker = AddBroker(core_type_name, initString + " --federates 1");
                AddFederates (ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 5: //pairs of federates per core
        {
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ii += 2)
            {
                auto init = initString + " --federates " + std::to_string((ii < count - 1) ? 2 : 1);
                auto core = helicsCreateCore (core_type_name.c_str(), NULL, init.c_str());
                CE (helicsCoreGetIdentifier (core, tmp, HELICS_SIZE_MAX));
                CE (helicsFederateInfoSetCoreName (fi, tmp));

                auto name = name_prefix + std::to_string (ii + offset);
                CE (helicsFederateInfoSetFederateName (fi, name.c_str()));
                auto fed = std::make_shared<helics_federate> (ctor (fi));
                federates[ii + offset] = fed;
                federates_added.push_back(fed);
                if (ii + 1 < count)
                {
                    name = name_prefix + std::to_string(ii + offset + 1);
                    CE (helicsFederateInfoSetFederateName (fi, name.c_str()));
                    auto fed2 = std::make_shared<helics_federate> (ctor (fi));
                    federates[ii + offset + 1] = fed2;
                    federates_added.push_back(fed2);
                }
            }
        }
            break;
        case 6: //pairs of cores per subbroker
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('5');
            for (int ii = 0; ii < count; ii+=4)
            {
                int fedcnt = (ii > count - 3) ? 4 : (count - ii);
                auto subbroker = AddBroker(core_type_name, initString + " --federates "+std::to_string(fedcnt));
                AddFederates (ctor, newTypeString, fedcnt, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 7: //two layers of subbrokers
        {      
                auto newTypeString = core_type_name;
                newTypeString.push_back('_');
                newTypeString.push_back('4');
                for (int ii = 0; ii < count; ++ii)
                {
                    auto subbroker = AddBroker(core_type_name, initString + " --federates 1");
                    AddFederates (ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
                }
        }
        break;
        }

        return federates_added;
    }

    helics_federate GetFederateAt (int index)
    {
        return federates.at(index).get();
    }

    std::vector<std::shared_ptr<helics_broker>> brokers;
    std::vector<std::shared_ptr<helics_federate>> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
    auto AddBrokerImp (const std::string &core_type_name, const std::string &initialization_string);
};