/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2012 Queen Mary, University of London.
    
    This file is part of vamp-aubio-plugins.

    vamp-aubio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    vamp-aubio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Types.h"

const char *getAubioNameForOnsetType(OnsetType t)
{
    // In the same order as the enum elements in the header
    static const char *const names[] = {
        "energy", "specdiff", "hfc", "complex", "phase", "kl", "mkl", "specflux", "default"
    };
    return names[(int)t];
}

const char *getAubioNameForSpecDescType(SpecDescType t)
{
    // In the same order as the enum elements in the header
    static const char *const names[] = {
        "specflux", "centroid", "spread", "skewness", "kurtosis", "slope", "decrease", "rolloff"
    };
    return names[(int)t];
}

const char *getAubioNameForPitchType(PitchType t)
{
    // In the same order as the enum elements in the header
    static const char *const names[] = {
        "yin", "mcomb", "schmitt", "fcomb", "yinfft"
    };
    return names[(int)t];
}

