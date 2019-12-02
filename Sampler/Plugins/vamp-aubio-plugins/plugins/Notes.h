/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
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

#ifndef _NOTES_PLUGIN_H_
#define _NOTES_PLUGIN_H_

#include <vamp-sdk/Plugin.h>
#include <aubio/aubio.h>

#include <deque>

#include "Types.h"

class Notes : public Vamp::Plugin
{
public:
    Notes(float inputSampleRate);
    virtual ~Notes();

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    InputDomain getInputDomain() const { return TimeDomain; }

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(std::string) const;
    void setParameter(std::string, float);

    size_t getPreferredStepSize() const;
    size_t getPreferredBlockSize() const;

    OutputList getOutputDescriptors() const;

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    fvec_t *m_ibuf;
    fvec_t *m_onset;
    fvec_t *m_pitch;
    aubio_onset_t *m_onsetdet;
    OnsetType m_onsettype;
    aubio_pitch_t *m_pitchdet;
    PitchType m_pitchtype;
    float m_threshold;
    float m_silence;
    float m_minioi;
    size_t m_median;
    size_t m_stepSize;
    size_t m_blockSize;
    int m_minpitch;
    int m_maxpitch;
    bool m_wrapRange;
    bool m_avoidLeaps;
    std::deque<float> m_notebuf;
    size_t m_count;
    Vamp::RealTime m_delay;
    Vamp::RealTime m_currentOnset;
    Vamp::RealTime m_lastTimeStamp;
    float m_currentLevel;
    bool m_haveCurrent;
    int m_prevPitch;

    void pushNote(FeatureSet &, const Vamp::RealTime &);
};


#endif
