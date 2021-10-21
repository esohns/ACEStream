/***************************************************************************
*   Copyright (C) 2009 by Erik Sohns   *
*   erik.sohns@web.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef STREAM_STAT_DEFINES_H
#define STREAM_STAT_DEFINES_H

#define MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING                "StatisticAnalysis"
#define MODULE_STAT_REPORT_DEFAULT_NAME_STRING                  "StatisticReport"

#define MODULE_STAT_ANALYSIS_DEFAULT_BUFFER_SIZE                1024 // #slots

// *NOTE*: (in a normal distribution,) values in the range of +/- 5.0 * sigma
//         (i.e. five standard deviations) account for 99.99994% of all sample
//         data (which is assumed to be static)
//         --> values outside of this range are potential 'activity' candidates
#define MODULE_STAT_ANALYSIS_ACTIVITY_DETECTION_DEVIATION_RANGE 5.0 // sigma
// *NOTE*: (in a normal distribution,) values in the range of +/- 6.0 * sigma
//         (i.e. six standard deviations) account for 99.999996% of all sample
//         data (which is assumed to be static)
//         --> values outside of this range are 'outliers' and hence potential
//             'peak' candidates
#define MODULE_STAT_ANALYSIS_PEAK_DETECTION_DEVIATION_RANGE     6.0 // sigma

#define MODULE_STAT_SPECTRUMANALYSIS_DEFAULT_SAMPLE_RATE        44100

#endif
