// Filename: msvAppInitializer.C
// Created on 19 Aug 2011 by Boyce Griffith
//
// Copyright (c) 2002-2010, Boyce Griffith
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of New York University nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "msvAppInitializer.h"

/////////////////////////////// INCLUDES /////////////////////////////////////

#ifndef included_IBTK_config
// #include <IBTK_config.h>
#define included_IBTK_config
#endif

#ifndef included_SAMRAI_config
#include <SAMRAI_config.h>
#define included_SAMRAI_config
#endif

// IBTK INCLUDES
#include <ibtk/namespaces.h>

// SAMRAI INCLUDES
#include <tbox/InputManager.h>
#include <tbox/NullDatabase.h>

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBTK
{
/////////////////////////////// STATIC ///////////////////////////////////////

/////////////////////////////// PUBLIC ///////////////////////////////////////

msvAppInitializer::msvAppInitializer(
  const std::string &input_filename,
  const std::string& default_log_file_name)
{
  // Create input database and parse all data in input file.
  d_input_db = new InputDatabase("input_db");
  InputManager::getManager()->parseInputFile(input_filename, d_input_db);

  // Process "Main" section of the input database.
  Pointer<Database> main_db = new NullDatabase();
  if (d_input_db->isDatabase("Main"))
  {
    main_db = d_input_db->getDatabase("Main");
  }

  // Configure logging options.
  const std::string log_file_name = main_db->getStringWithDefault("log_file_name", default_log_file_name);
  const bool log_all_nodes = main_db->getBoolWithDefault("log_all_nodes", false);
  if (!log_file_name.empty())
  {
    if (log_all_nodes)
    {
      PIO::logAllNodes(log_file_name);
    }
    else
    {
      PIO::logOnlyNodeZero(log_file_name);
    }
  }

  return;
}// msvAppInitializer

msvAppInitializer::~msvAppInitializer()
{
    InputManager::freeManager();
    return;
}// ~msvAppInitializer

Pointer<Database>
msvAppInitializer::getInputDatabase()
{
    return d_input_db;
}// getInputDatabase

bool
msvAppInitializer::isFromRestart() const
{
    return d_is_from_restart;
}// isFromRestart

Pointer<Database>
msvAppInitializer::getRestartDatabase(
    const bool suppress_warning)
{
    if (!d_is_from_restart && !suppress_warning)
    {
        pout << "WARNING: msvAppInitializer::getRestartDatabase(): Not a restarted run, restart database is empty\n";
    }
    return RestartManager::getManager()->getRootDatabase();
}// getRestartDatabase

Pointer<Database>
msvAppInitializer::getComponentDatabase(
    const std::string& component_name,
    const bool suppress_warning)
{
    const bool db_exists = d_input_db->isDatabase(component_name);
    if (!db_exists && !suppress_warning)
    {
        pout << "WARNING: msvAppInitializer::getComponentDatabase(): Database corresponding to component `" << component_name << "' not found in input\n";
        return new NullDatabase();
    }
    else
    {
        return d_input_db->getDatabase(component_name);
    }
}// getComponentDatabase

bool
msvAppInitializer::dumpVizData() const
{
    return d_viz_dump_interval > 0;
}// dumpVizData

int
msvAppInitializer::getVizDumpInterval() const
{
    return d_viz_dump_interval;
}// getVizDumpInterval

std::string
msvAppInitializer::getVizDumpDirectory() const
{
    return d_viz_dump_dirname;
}// getVizDumpDirectory

std::vector<std::string>
msvAppInitializer::getVizWriters() const
{
    return d_viz_writers;
}// getVizDumpDirectory

Pointer<VisItDataWriter<NDIM> >
msvAppInitializer::getVisItDataWriter() const
{
    return d_visit_data_writer;
}// getVisItDataWriter

Pointer<LSiloDataWriter>
msvAppInitializer::getLSiloDataWriter() const
{
    return d_silo_data_writer;
}// getLSiloDataWriter

std::string
msvAppInitializer::getExodusIIFilename(
    const std::string& prefix) const
{
    std::string exodus_filename = "";
    if (!d_exodus_filename.empty())
    {
        std::ostringstream exodus_filename_stream;
        exodus_filename_stream << d_viz_dump_dirname << "/" << prefix << d_exodus_filename;
        exodus_filename = exodus_filename_stream.str();
    }
    return exodus_filename;
}// getExodusIIFilename

bool
msvAppInitializer::dumpRestartData() const
{
    return d_restart_dump_interval > 0;
}// dumpRestartData

int
msvAppInitializer::getRestartDumpInterval() const
{
    return d_restart_dump_interval;
}// getRestartDumpInterval

std::string
msvAppInitializer::getRestartDumpDirectory() const
{
    return d_restart_dump_dirname;
}// getRestartDumpDirectory

bool
msvAppInitializer::dumpPostProcessingData() const
{
    return d_data_dump_interval > 0;
}// dumpPostProcessingData

int
msvAppInitializer::getPostProcessingDataDumpInterval() const
{
    return d_data_dump_interval;
}// getPostProcessingDataDumpInterval

std::string
msvAppInitializer::getPostProcessingDataDumpDirectory() const
{
    return d_data_dump_dirname;
}// getPostProcessingDataDumpDirectory

bool
msvAppInitializer::dumpTimerData() const
{
    return d_timer_dump_interval > 0;
}// dumpTimerData

int
msvAppInitializer::getTimerDumpInterval() const
{
    return d_timer_dump_interval;
}// getTimerDumpInterval

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

/////////////////////////////// NAMESPACE ////////////////////////////////////

} // namespace IBTK

//////////////////////////////////////////////////////////////////////////////
