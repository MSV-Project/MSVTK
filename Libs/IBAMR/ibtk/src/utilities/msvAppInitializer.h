/*==============================================================================

  Library: MSVTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/
// Filename: AppInitializer.h
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

#ifndef included_msvAppInitializer
#define included_msvAppInitializer

/////////////////////////////// INCLUDES /////////////////////////////////////

// IBTK INCLUDES
#include <ibtk/LSiloDataWriter.h>

// SAMRAI INCLUDES
#include <VisItDataWriter.h>

/////////////////////////////// CLASS DEFINITION /////////////////////////////

namespace IBTK
{
  /*!
   * \brief Class AppInitializer provides functionality to simplify the
   * initialization code in an application code.
   */
  class msvAppInitializer
  : public SAMRAI::tbox::DescribedClass
  {
  public:

    /*!
     * Constructor for class msvAppInitializer parses command line arguments, sets
     * up input and restart databases, and enables SAMRAI logging.
     */
    msvAppInitializer(
      const std::string &filename,
      const std::string& default_log_file_name="IBAMR.log");

    /*!
     * Destructor for class AppInitializer frees the SAMRAI manager objects
     * used to set up input and restart databases.
     */
    ~msvAppInitializer();

    /*!
     * Return a pointer to the input database.
     */
    SAMRAI::tbox::Pointer<SAMRAI::tbox::Database>
    getInputDatabase();

    /*!
     * Return a boolean value indicating whether this is a restarted run.
     */
    bool
    isFromRestart() const;

    /*!
     * Return a pointer to the restart database.  If there is no restart
     * database for the application, this method emits a warning message and
     * returns a NullDatabse.
     */
    SAMRAI::tbox::Pointer<SAMRAI::tbox::Database>
    getRestartDatabase(
      bool suppress_warning=false);

    /*!
     * Return initialization database for the requested solver component.  This
     * is equivalent to:
     * getInputDatabase()->getDatabase(component_name).
     *
     * If the requested component is not found in the input database, this
     * method emits a warning message and returns a NullDatabse.
     */
    SAMRAI::tbox::Pointer<SAMRAI::tbox::Database>
    getComponentDatabase(
      const std::string& component_name,
      bool suppress_warning=false);

    /*!
     * Return a boolean value indicating whether to write visualization data.
     */
    bool
    dumpVizData() const;

    /*!
     * Return the visualization dump interval.
     */
    int
    getVizDumpInterval() const;

    /*!
     * Return the visualization dump directory name.
     */
    std::string
    getVizDumpDirectory() const;

    /*!
     * Return the visualization writers to be used in the simulation.
     */
    std::vector<std::string>
    getVizWriters() const;

    /*!
     * Return a VisIt data writer object to be used to output Cartesian grid
     * data.
     *
     * If the application is not configured to use VisIt, a NULL pointer will be
     * returned.
     */
    SAMRAI::tbox::Pointer<SAMRAI::appu::VisItDataWriter<NDIM> >
    getVisItDataWriter() const;

    /*!
     * Return a VisIt data writer object to be used to output Lagrangian data.
     *
     * If the application is not configured to use VisIt, a NULL pointer will be
     * returned.
     */
    SAMRAI::tbox::Pointer<LSiloDataWriter>
    getLSiloDataWriter() const;

    /*!
     * Return the ExodusII visualization file name.
     *
     * If the application is not configured to use ExodusII, an empty string
     * will be returned.
     */
    std::string
    getExodusIIFilename(
      const std::string& prefix="") const;

      /*!
       * Return a boolean value indicating whether to write restart data.
       */
      bool
      dumpRestartData() const;

      /*!
       * Return the restart dump interval.
       */
      int
      getRestartDumpInterval() const;

      /*!
       * Return the restart dump directory name.
       */
      std::string
      getRestartDumpDirectory() const;

      /*!
       * Return a boolean value indicating whether to write post processing data.
       */
      bool
      dumpPostProcessingData() const;

      /*!
       * Return the post processing data dump interval.
       */
      int
      getPostProcessingDataDumpInterval() const;

      /*!
       * Return the post processing data dump directory name.
       */
      std::string
      getPostProcessingDataDumpDirectory() const;

      /*!
       * Return a boolean value indicating whether to write timer data.
       */
      bool
      dumpTimerData() const;

      /*!
       * Return the timer dump interval.
       */
      int
      getTimerDumpInterval() const;

  private:
    /*!
     * \brief Copy constructor.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param from The value to copy to this object.
     */
    msvAppInitializer(
      const msvAppInitializer& from);

    /*!
     * \brief Assignment operator.
     *
     * \note This operator is not implemented and should not be used.
     *
     * \param that The value to assign to this object.
     *
     * \return A reference to this object.
     */
    msvAppInitializer&
    operator=(
      const msvAppInitializer& that);

    /*!
     * The input database.
     */
    SAMRAI::tbox::Pointer<SAMRAI::tbox::Database> d_input_db;

    /*!
     * Boolean value indicating whether this is a restarted run.
     */
    bool d_is_from_restart;

    /*!
     * Visualization options.
     */
    int d_viz_dump_interval;
    std::string d_viz_dump_dirname;
    std::vector<std::string> d_viz_writers;
    SAMRAI::tbox::Pointer<SAMRAI::appu::VisItDataWriter<NDIM> > d_visit_data_writer;
    SAMRAI::tbox::Pointer<LSiloDataWriter> d_silo_data_writer;
    std::string d_exodus_filename;

    /*!
     * Restart options.
     */
    int d_restart_dump_interval;
    std::string d_restart_dump_dirname;

    /*!
     * Post-processing options.
     */
    int d_data_dump_interval;
    std::string d_data_dump_dirname;

    /*!
     * Timer options.
     */
    int d_timer_dump_interval;
  };
}// namespace IBTK

/////////////////////////////// INLINE ///////////////////////////////////////

//#include <ibtk/AppInitializer.I>

//////////////////////////////////////////////////////////////////////////////

#endif //#ifndef included_AppInitializer
