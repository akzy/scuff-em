/* Copyright (C) 2005-2011 M. T. Homer Reid
 *
 * This file is part of SCUFF-EM.
 *
 * SCUFF-EM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SCUFF-EM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * scuff-heat.cc  -- a standalone code within the scuff-EM suite for 
 *                -- solving problems involving the heat radiation
 *                -- of a single object or the heat transfer from
 *                -- one object to another object
 *
 * homer reid     -- 2/2012
 *
 * --------------------------------------------------------------
 *
 * this program has a large number of command-line options, which
 * subdivide into a number of categories as described below.
 *
 * in addition to the command line, options may also be specified
 * in an input file, piped into standard input with one option-value
 * pair per line; thus, if --option1 and --option2 are command-line 
 * options as described below, then you may create an input file 
 * (call it 'myOptions') with the content 
 *                  
 *   ...
 *   option1 value1
 *   option2 value2
 *   ...
 * 
 * and then running 
 * 
 *  scuff-heat < myOptions 
 * 
 * is equivalent to 
 * 
 *  scuff-heat --option1 value1 --option value2.
 * 
 * (if any options are specified both on standard input and 
 *  on the command line, the values given on the command line take
 *  precedence.)
 * 
 *       -------------------------------------------------
 * 
 * a. options describing the geometry and the (optional)
 *    list of geometrical transformations applied to it
 * 
 *     --geometry  MyGeometry.scuffgeo
 *     --transfile MyGeometry.trans
 * 
 *       -------------------------------------------------
 * 
 * b. options describing the frequency
 * 
 *     --Omega xx
 *
 *         Specify a single frequency at which to compute 
 *         the spectral density of heat transfer.
 *
 *         Note that --Omega may be specified more than 
 *         once.
 * 
 *     --OmegaFile MyOmegaFile
 * 
 *         Specify a file containing a list of --Omega values.
 * 
 *     --OmegaMin xx
 *     --OmegaMax xx
 * 
 *         Specify a range of frequencies over which to 
 *         integrate the spectral density to compute 
 *         a total (integrated) heat transfer.
 * 
 *     Note: if no frequency options are specified, the 
 *           default behavior is to integrate over the  
 *           entire positive real frequency axis. In other
 *           words, specifying no frequency options is 
 *           equivalent to setting OmegaMin=0.0 and 
 *           OmegaMax=infinity.
 * 
 *       -------------------------------------------------
 * 
 * c. options specifying output file names 
 * 
 *     --ByOmegaFile .byOmega
 * 
 *         Set the name of the frequency-resolved output  
 *         file. If this option is not specified, the 
 *         frequency-resolved output file will be called
 *         Geometry.byOmega, where Geometry.scuffgeo is the
 *         geometry file specified using the --geometry option.
 * 
 *     --OutputFile MyFile.Out
 * 
 *         Set the name of the total-integrated-power output  
 *         file (which is only generated if your command-line
 *         options call for scuff-heat to evaluate a frequency
 *         integral.)
 *         If this option is not specified, the output file 
 *         will be called Geometry.out, where Geometry.scuffgeo 
 *         is the geometry file specified using the --geometry 
 *         option.
 * 
 *     --LogFile MyFile.log
 * 
 *         Set the name of the log file. If this option is not
 *         specified, the log file will be named scuff-heat.log.
 * 
 * d. options specifying scuff caches 
 * 
 *       -------------------------------------------------
 * 
 *     -- ReadCache MyReadCache.scuffcache
 * 
 *         Specify a scuff cache file to be preloaded.
 *         This option may be specified more than once. 
 * 
 *     -- WriteCache MyWriteCache.scuffcache
 * 
 *         Specify the name of a scuff cache file to which
 *         scuff-heat will dump the contents of its cache
 *         after completing all computations.
 * 
 *     -- Cache MyCache.scuffcache
 * 
 *         Specify a cache file that scuff-heat will both
 *         (a) preload before starting its computations, and
 *         (b) overwrite after completing its computations.
 *         Specifying this option is equivalent to setting
 *         --ReadCache and --WriteCache both to MyCache.scuffcache.
 * 
 * e. other options 
 * 
 *     --nThread xx   (use xx computational threads)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "scuff-heat.h"

/***************************************************************/
/***************************************************************/
/***************************************************************/
#define MAXFREQ  10    // max number of frequencies 
#define MAXCACHE 10    // max number of cache files for preload

#define MAXSTR   1000

/***************************************************************/
/***************************************************************/
/***************************************************************/
int main(int argc, char *argv[])
{
  InstallHRSignalHandler();

  /***************************************************************/
  /* process options *********************************************/
  /***************************************************************/
  char *GeoFile=0;
  char *TransFile=0;
  cdouble OmegaMin=0.0;              int nOmegaMin;
  cdouble OmegaMax=-1.0;             int nOmegaMax;
  cdouble OmegaVals[MAXFREQ];        int nOmegaVals;
  char *OmegaFile;                   int nOmegaFiles;
  char *ByOmegaFile=0;
  int PlotFlux=0;
  char *OutputFile=0;
  char *LogFile=0;
  char *Cache=0;
  char *ReadCache[MAXCACHE];         int nReadCache;
  char *WriteCache=0;
  double SWPPITol=0.0;
  int nThread=0;
  /* name               type    #args  max_instances  storage           count         description*/
  OptStruct OSArray[]=
   { {"Geometry",       PA_STRING,  1, 1,       (void *)&GeoFile,    0,             "geometry file"},
     {"TransFile",      PA_STRING,  1, 1,       (void *)&TransFile,  0,             "list of geometrical transformation"},
     {"Omega",          PA_CDOUBLE, 1, MAXFREQ, (void *)OmegaVals,   &nOmegaVals,   "(angular) frequency"},
     {"OmegaFile",      PA_STRING,  1, 1,       (void *)&OmegaFile,  &nOmegaFiles,  "list of (angular) frequencies"},
     {"OmegaMin",       PA_CDOUBLE, 1, 1,       (void *)&OmegaMin,   &nOmegaMin,    "lower integration limit"},
     {"OmegaMax",       PA_CDOUBLE, 1, 1,       (void *)&OmegaMax,   &nOmegaMax,    "upper integration limit"},
     {"OutputFile",     PA_STRING,  1, 1,       (void *)&OutputFile, 0,             "name of frequency-integrated output file"},
     {"ByOmegaFile",    PA_STRING,  1, 1,       (void *)&ByOmegaFile,0,             "name of frequency-resolved output file"},
     {"PlotFlux",       PA_BOOL,    0, 1,       (void *)&PlotFlux,   0,             "write spatially-resolved flux data"},
     {"LogFile",        PA_STRING,  1, 1,       (void *)&LogFile,    0,             "name of log file"},
     {"Cache",          PA_STRING,  1, 1,       (void *)&Cache,      0,             "read/write cache"},
     {"ReadCache",      PA_STRING,  1, MAXCACHE,(void *)ReadCache,   &nReadCache,   "read cache"},
     {"WriteCache",     PA_STRING,  1, 1,       (void *)&WriteCache, 0,             "write cache"},
     {"nThread",        PA_INT,     1, 1,       (void *)&nThread,    0,             "number of CPU threads to use"},
     {0,0,0,0,0,0,0}
   };
  ProcessOptions(argc, argv, OSArray);

  if (GeoFile==0)
   OSUsage(argv[0], OSArray, "--geometry option is mandatory");

  if ( Cache!=0 && WriteCache!=0 )
   ErrExit("--cache and --writecache options are mutually exclusive");

  if (PlotFlux && ByOmegaFile!=0 )
   ErrExit("--PlotFlux and --ByOmegaFile options are mutually exclusive");

  if (LogFile)
   SetLogFileName(LogFile);
  else
   SetLogFileName("scuff-heat.log");

  Log("scuff-heat running on %s",GetHostName());

  /*******************************************************************/
  /* process frequency-related options to construct a list of        */
  /* frequencies at which to run simulations                         */
  /*******************************************************************/
  HVector *OmegaList=0, *OmegaList0;
  int nFreq, nOV, NumFreqs=0;
  if (nOmegaFiles==1) // first process --OmegaFile option if present
   { 
     OmegaList=new HVector(OmegaFile,LHM_TEXT);
     if (OmegaList->ErrMsg)
      ErrExit(OmegaList->ErrMsg);
     NumFreqs=OmegaList->N;
     Log("Read %i frequencies from file %s.",NumFreqs,OmegaFile);
   };

  // now add any individually specified --Omega options
  if (nOmegaVals>0)
   { 
     NumFreqs += nOmegaVals;
     OmegaList0=OmegaList;
     OmegaList=new HVector(NumFreqs, LHM_COMPLEX);
     nFreq=0;
     if (OmegaList0)
      { for(nFreq=0; nFreq<OmegaList0->N; nFreq++)
         OmegaList->SetEntry(nFreq, OmegaList0->GetEntry(nFreq));
        delete OmegaList0;
      };
     for(nOV=0; nOV<nOmegaVals; nOV++)
      OmegaList->SetEntry(nFreq+nOV, OmegaVals[nOV]);
     Log("Read %i frequencies from command line.",nOmegaVals);
   };

  // check that the user didn't simultaneously ask for a discrete
  // list of frequencies and a frequency range over which to integrate;
  // if a range was specified check that it makes sense 
  if ( NumFreqs>0 ) 
   { if ( nOmegaMin>0 || nOmegaMax>0 )
      ErrExit("--OmegaMin/--OmegaMax options may not be used with --Omega/--OmegaFile");  
     Log("Computing spectral density at %i frequencies.",NumFreqs);
   }
  else if (NumFreqs==0)
   { 
     // if --OmegaMin and/or --OmegaMax values were specified,
     // check that they make sense
     if ( nOmegaMin==1 && (real(OmegaMin)<0.0 || imag(OmegaMin)!=0.0) )
      ErrExit("invalid value specified for --OmegaMin");
     if ( nOmegaMax==1 && (real(OmegaMax)<real(OmegaMin) || imag(OmegaMax)!=0.0 ) )
      ErrExit("invalid value specified for --OmegaMax");

     if ( OmegaMax==-1.0 )
      Log("Integrating over range Omega=(%g,infinity).",real(OmegaMin));
     else
      Log("Integrating over range Omega=(%g,%g).",real(OmegaMin),real(OmegaMax));
   };

  /*******************************************************************/
  /* create the SHData structure that contains all the info needed   */
  /* to evaluate the heat transfer at a single frequency             */
  /*******************************************************************/
  SHData *SHD=CreateSHData(GeoFile, TransFile, PlotFlux, ByOmegaFile, nThread);

  /*******************************************************************/
  /* preload the scuff cache with any cache preload files the user   */
  /* may have specified                                              */
  /*******************************************************************/
  for (int nrc=0; nrc<nReadCache; nrc++)
   PreloadCache( ReadCache[nrc] );
  if (Cache)
   PreloadCache( Cache );

  if (Cache) WriteCache=Cache;
  SHD->WriteCache = WriteCache;

  /*******************************************************************/
  /* now switch off based on the requested frequency behavior to     */
  /* perform the actual calculations                                 */
  /*******************************************************************/
  double *I = new double[SHD->NumTransformations];
  if (NumFreqs>0)
   { for (nFreq=0; nFreq<NumFreqs; nFreq++)
      GetFrequencyIntegrand(SHD, OmegaList->GetEntry(nFreq), I);
   }
  else
   { // frequency integration not yet implemented 
     ErrExit("frequency integration is not yet implemented");
   };
  delete[] I;

  /***************************************************************/
  /***************************************************************/
  /***************************************************************/
  printf("Thank you for your support.\n");

}
