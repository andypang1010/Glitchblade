/*******************************************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER $200k in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below $200k annual revenue or funding.

For entities with OVER $200k in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing@cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
*******************************************************************************************************************/

#include "RNBO_Common.h"
#include "RNBO_AudioSignal.h"

namespace RNBO {


#define trunc(x) ((Int)(x))

#if defined(__GNUC__) || defined(__clang__)
    #define RNBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
    #define RNBO_RESTRICT __restrict
#endif

#define FIXEDSIZEARRAYINIT(...) { }

class rnbomatic : public PatcherInterfaceImpl {
public:

rnbomatic()
{
}

~rnbomatic()
{
}

rnbomatic* getTopLevelPatcher() {
    return this;
}

void cancelClockEvents()
{
    getEngine()->flushClockEvents(this, 1696335371, false);
    getEngine()->flushClockEvents(this, -1735137014, false);
    getEngine()->flushClockEvents(this, -871642103, false);
    getEngine()->flushClockEvents(this, 1673828112, false);
    getEngine()->flushClockEvents(this, -1214668403, false);
    getEngine()->flushClockEvents(this, -30654451, false);
    getEngine()->flushClockEvents(this, 1935387534, false);
    getEngine()->flushClockEvents(this, 1634454928, false);
    getEngine()->flushClockEvents(this, -1586675634, false);
    getEngine()->flushClockEvents(this, -1757644273, false);
    getEngine()->flushClockEvents(this, 1262447697, false);
    getEngine()->flushClockEvents(this, 890440466, false);
    getEngine()->flushClockEvents(this, 832840460, false);
    getEngine()->flushClockEvents(this, 518433235, false);
    getEngine()->flushClockEvents(this, -1172483444, false);
    getEngine()->flushClockEvents(this, 1592269969, false);
    getEngine()->flushClockEvents(this, -1958682865, false);
    getEngine()->flushClockEvents(this, -1544490675, false);
    getEngine()->flushClockEvents(this, 1220262738, false);
    getEngine()->flushClockEvents(this, -1916497906, false);
    getEngine()->flushClockEvents(this, 2006462159, false);
    getEngine()->flushClockEvents(this, -871550838, false);
    getEngine()->flushClockEvents(this, 1964277200, false);
    getEngine()->flushClockEvents(this, 146426004, false);
    getEngine()->flushClockEvents(this, 848255507, false);
    getEngine()->flushClockEvents(this, -894149362, false);
    getEngine()->flushClockEvents(this, -1357044121, false);
    getEngine()->flushClockEvents(this, -1243558069, false);
    getEngine()->flushClockEvents(this, -225581227, false);
    getEngine()->flushClockEvents(this, -1615565300, false);
    getEngine()->flushClockEvents(this, -470653941, false);
    getEngine()->flushClockEvents(this, -1987572531, false);
    getEngine()->flushClockEvents(this, -842661172, false);
    getEngine()->flushClockEvents(this, -1468824490, false);
}

template <typename T> void listquicksort(T& arr, T& sortindices, Int l, Int h, bool ascending) {
    if (l < h) {
        Int p = (Int)(this->listpartition(arr, sortindices, l, h, ascending));
        this->listquicksort(arr, sortindices, l, p - 1, ascending);
        this->listquicksort(arr, sortindices, p + 1, h, ascending);
    }
}

template <typename T> Int listpartition(T& arr, T& sortindices, Int l, Int h, bool ascending) {
    number x = arr[(Index)h];
    Int i = (Int)(l - 1);

    for (Int j = (Int)(l); j <= h - 1; j++) {
        bool asc = (bool)((bool)(ascending) && arr[(Index)j] <= x);
        bool desc = (bool)((bool)(!(bool)(ascending)) && arr[(Index)j] >= x);

        if ((bool)(asc) || (bool)(desc)) {
            i++;
            this->listswapelements(arr, i, j);
            this->listswapelements(sortindices, i, j);
        }
    }

    i++;
    this->listswapelements(arr, i, h);
    this->listswapelements(sortindices, i, h);
    return i;
}

template <typename T> void listswapelements(T& arr, Int a, Int b) {
    auto tmp = arr[(Index)a];
    arr[(Index)a] = arr[(Index)b];
    arr[(Index)b] = tmp;
}

number mstosamps(MillisecondTime ms) {
    return ms * this->sr * 0.001;
}

number samplerate() const {
    return this->sr;
}

Index vectorsize() const {
    return this->vs;
}

inline number safemod(number f, number m) {
    if (m != 0) {
        Int f_trunc = (Int)(trunc(f));
        Int m_trunc = (Int)(trunc(m));

        if (f == f_trunc && m == m_trunc) {
            f = f_trunc % m_trunc;
        } else {
            if (m < 0) {
                m = -m;
            }

            if (f >= m) {
                if (f >= m * 2.0) {
                    number d = f / m;
                    Int i = (Int)(trunc(d));
                    d = d - i;
                    f = d * m;
                } else {
                    f -= m;
                }
            } else if (f <= -m) {
                if (f <= -m * 2.0) {
                    number d = f / m;
                    Int i = (Int)(trunc(d));
                    d = d - i;
                    f = d * m;
                } else {
                    f += m;
                }
            }
        }
    } else {
        f = 0.0;
    }

    return f;
}

inline number safesqrt(number num) {
    return (num > 0.0 ? rnbo_sqrt(num) : 0.0);
}

number minimum(number x, number y) {
    return (y < x ? y : x);
}

number maximum(number x, number y) {
    return (x < y ? y : x);
}

inline number safediv(number num, number denom) {
    return (denom == 0.0 ? 0.0 : num / denom);
}

number safepow(number base, number exponent) {
    return fixnan(rnbo_pow(base, exponent));
}

number scale(
    number x,
    number lowin,
    number hiin,
    number lowout,
    number highout,
    number pow
) {
    auto inscale = this->safediv(1., hiin - lowin);
    number outdiff = highout - lowout;
    number value = (x - lowin) * inscale;

    if (pow != 1) {
        if (value > 0)
            value = this->safepow(value, pow);
        else
            value = -this->safepow(-value, pow);
    }

    value = value * outdiff + lowout;
    return value;
}

Index voice() {
    return this->_voiceIndex;
}

number random(number low, number high) {
    number range = high - low;
    return globalrandom() * range + low;
}

MillisecondTime currenttime() {
    return this->_currentTime;
}

number __wrapped_op_clamp(number in1, number in2, number in3) {
    return (in1 > in3 ? in3 : (in1 < in2 ? in2 : in1));
}

inline number linearinterp(number frac, number x, number y) {
    return x + (y - x) * frac;
}

number wrap(number x, number low, number high) {
    number lo;
    number hi;

    if (low == high)
        return low;

    if (low > high) {
        hi = low;
        lo = high;
    } else {
        lo = low;
        hi = high;
    }

    number range = hi - lo;

    if (x >= lo && x < hi)
        return x;

    if (range <= 0.000000001)
        return lo;

    long numWraps = (long)(trunc((x - lo) / range));
    numWraps = numWraps - ((x < lo ? 1 : 0));
    number result = x - range * numWraps;

    if (result >= hi)
        return result - range;
    else
        return result;
}

number tempo() {
    return this->getTopLevelPatcher()->globaltransport_getTempo(this->currenttime());
}

number tickstoms(number ticks) {
    return ticks / (number)480 * this->safediv(60, this->tempo()) * 1000;
}

MillisecondTime sampstoms(number samps) {
    return samps * 1000 / this->sr;
}

Index getNumMidiInputPorts() const {
    return 0;
}

void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}

Index getNumMidiOutputPorts() const {
    return 0;
}

void process(
    const SampleValue * const* inputs,
    Index numInputs,
    SampleValue * const* outputs,
    Index numOutputs,
    Index n
) {
    RNBO_UNUSED(numInputs);
    RNBO_UNUSED(inputs);
    this->vs = n;
    this->updateTime(this->getEngine()->getCurrentTime());
    SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
    SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);

    this->groove_01_perform(
        this->groove_01_rate_auto,
        this->groove_01_begin,
        this->groove_01_end,
        this->signals[0],
        this->dummyBuffer,
        n
    );

    this->gaintilde_02_perform(this->signals[0], this->signals[1], n);
    this->gaintilde_01_perform(this->signals[0], this->signals[2], n);

    this->groove_02_perform(
        this->groove_02_rate_auto,
        this->groove_02_begin,
        this->groove_02_end,
        this->signals[0],
        this->dummyBuffer,
        n
    );

    this->gaintilde_03_perform(this->signals[0], this->signals[3], n);
    this->gaintilde_04_perform(this->signals[0], this->signals[4], n);

    this->groove_03_perform(
        this->groove_03_rate_auto,
        this->groove_03_begin,
        this->groove_03_end,
        this->signals[0],
        this->dummyBuffer,
        this->dummyBuffer,
        n
    );

    this->pan_tilde_02_perform(
        this->pan_tilde_02_pos,
        this->signals[0],
        this->signals[5],
        this->signals[6],
        n
    );

    this->dspexpr_01_perform(this->signals[5], this->dspexpr_01_in2, this->signals[0], n);
    this->dspexpr_03_perform(this->signals[6], this->dspexpr_03_in2, this->signals[5], n);

    this->adsr_01_perform(
        this->adsr_01_attack,
        this->adsr_01_decay,
        this->adsr_01_sustain,
        this->adsr_01_release,
        this->zeroBuffer,
        this->dummyBuffer,
        n
    );

    this->mtof_tilde_01_perform(this->mtof_tilde_01_midivalue, this->signals[6], n);

    this->cycle_tilde_01_perform(
        this->signals[6],
        this->cycle_tilde_01_phase_offset,
        this->signals[7],
        this->dummyBuffer,
        n
    );

    this->dspexpr_02_perform(this->signals[7], this->dspexpr_02_in2, this->signals[6], n);

    this->pan_tilde_01_perform(
        this->pan_tilde_01_pos,
        this->signals[6],
        this->signals[7],
        this->signals[8],
        n
    );

    this->mtof_tilde_02_perform(this->mtof_tilde_02_midivalue, this->signals[6], n);

    this->rect_tilde_01_perform(
        this->signals[6],
        this->rect_tilde_01_pulsewidth,
        this->rect_tilde_01_syncPhase,
        this->signals[9],
        this->dummyBuffer,
        n
    );

    this->dspexpr_04_perform(this->signals[9], this->dspexpr_04_in2, this->signals[6], n);

    this->pan_tilde_03_perform(
        this->pan_tilde_03_pos,
        this->signals[6],
        this->signals[9],
        this->signals[10],
        n
    );

    this->dspexpr_05_perform(this->dspexpr_05_in1, this->dspexpr_05_in2, this->dummyBuffer, n);
    this->mtof_tilde_03_perform(this->mtof_tilde_03_midivalue, this->signals[6], n);

    this->saw_tilde_01_perform(
        this->signals[6],
        this->saw_tilde_01_syncPhase,
        this->signals[11],
        this->dummyBuffer,
        n
    );

    this->dspexpr_06_perform(this->signals[11], this->dspexpr_06_in2, this->signals[6], n);

    this->pan_tilde_04_perform(
        this->pan_tilde_04_pos,
        this->signals[6],
        this->signals[11],
        this->signals[12],
        n
    );

    this->mtof_tilde_04_perform(this->mtof_tilde_04_midivalue, this->signals[6], n);
    this->phasor_01_perform(this->signals[6], this->signals[13], n);
    this->triangle_tilde_01_perform(this->signals[13], this->triangle_tilde_01_duty, this->signals[6], n);
    this->dspexpr_07_perform(this->signals[6], this->dspexpr_07_in2, this->signals[13], n);

    this->pan_tilde_06_perform(
        this->pan_tilde_06_pos,
        this->signals[13],
        this->signals[6],
        this->signals[14],
        n
    );

    this->signaladder_01_perform(
        this->signals[6],
        this->signals[11],
        this->signals[9],
        this->signals[7],
        this->signals[7],
        n
    );

    this->gaintilde_07_perform(this->signals[7], this->signals[13], n);

    this->signaladder_02_perform(
        this->signals[14],
        this->signals[12],
        this->signals[10],
        this->signals[8],
        this->signals[8],
        n
    );

    this->gaintilde_08_perform(this->signals[8], this->signals[7], n);

    this->groove_04_perform(
        this->groove_04_rate_auto,
        this->groove_04_begin,
        this->groove_04_end,
        this->signals[8],
        this->dummyBuffer,
        n
    );

    this->pan_tilde_05_perform(
        this->pan_tilde_05_pos,
        this->signals[8],
        this->signals[10],
        this->signals[12],
        n
    );

    this->gaintilde_05_perform(this->signals[10], this->signals[8], n);

    this->signaladder_03_perform(
        this->signals[2],
        this->signals[0],
        this->signals[3],
        this->signals[8],
        this->signals[13],
        out1,
        n
    );

    this->gaintilde_06_perform(this->signals[12], this->signals[13], n);

    this->signaladder_04_perform(
        this->signals[1],
        this->signals[5],
        this->signals[4],
        this->signals[13],
        this->signals[7],
        out2,
        n
    );

    this->stackprotect_perform(n);
    this->globaltransport_advance();
    this->audioProcessSampleCount += this->vs;
}

void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
    if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
        Index i;

        for (i = 0; i < 15; i++) {
            this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
        }

        this->adsr_01_triggerBuf = resizeSignal(this->adsr_01_triggerBuf, this->maxvs, maxBlockSize);
        this->adsr_01_triggerValueBuf = resizeSignal(this->adsr_01_triggerValueBuf, this->maxvs, maxBlockSize);
        this->phasor_01_sigbuf = resizeSignal(this->phasor_01_sigbuf, this->maxvs, maxBlockSize);
        this->globaltransport_tempo = resizeSignal(this->globaltransport_tempo, this->maxvs, maxBlockSize);
        this->globaltransport_state = resizeSignal(this->globaltransport_state, this->maxvs, maxBlockSize);
        this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
        this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
        this->didAllocateSignals = true;
    }

    const bool sampleRateChanged = sampleRate != this->sr;
    const bool maxvsChanged = maxBlockSize != this->maxvs;
    const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;

    if (sampleRateChanged || maxvsChanged) {
        this->vs = maxBlockSize;
        this->maxvs = maxBlockSize;
        this->sr = sampleRate;
        this->invsr = 1 / sampleRate;
    }

    this->data_01_dspsetup(forceDSPSetup);
    this->data_02_dspsetup(forceDSPSetup);
    this->data_03_dspsetup(forceDSPSetup);
    this->data_04_dspsetup(forceDSPSetup);
    this->data_05_dspsetup(forceDSPSetup);
    this->data_06_dspsetup(forceDSPSetup);
    this->data_07_dspsetup(forceDSPSetup);
    this->data_08_dspsetup(forceDSPSetup);
    this->groove_01_dspsetup(forceDSPSetup);
    this->gaintilde_02_dspsetup(forceDSPSetup);
    this->gaintilde_01_dspsetup(forceDSPSetup);
    this->groove_02_dspsetup(forceDSPSetup);
    this->gaintilde_03_dspsetup(forceDSPSetup);
    this->gaintilde_04_dspsetup(forceDSPSetup);
    this->groove_03_dspsetup(forceDSPSetup);
    this->adsr_01_dspsetup(forceDSPSetup);
    this->cycle_tilde_01_dspsetup(forceDSPSetup);
    this->saw_tilde_01_dspsetup(forceDSPSetup);
    this->phasor_01_dspsetup(forceDSPSetup);
    this->gaintilde_07_dspsetup(forceDSPSetup);
    this->gaintilde_08_dspsetup(forceDSPSetup);
    this->groove_04_dspsetup(forceDSPSetup);
    this->gaintilde_05_dspsetup(forceDSPSetup);
    this->gaintilde_06_dspsetup(forceDSPSetup);
    this->globaltransport_dspsetup(forceDSPSetup);

    if (sampleRateChanged)
        this->onSampleRateChanged(sampleRate);
}

void setProbingTarget(MessageTag id) {
    switch (id) {
    default:
        {
        this->setProbingIndex(-1);
        break;
        }
    }
}

void setProbingIndex(ProbingIndex ) {}

Index getProbingChannels(MessageTag outletId) const {
    RNBO_UNUSED(outletId);
    return 0;
}

DataRef* getDataRef(DataRefIndex index)  {
    switch (index) {
    case 0:
        {
        return addressOf(this->background);
        break;
        }
    case 1:
        {
        return addressOf(this->b_hit1_wav);
        break;
        }
    case 2:
        {
        return addressOf(this->bigbot);
        break;
        }
    case 3:
        {
        return addressOf(this->littlebot);
        break;
        }
    case 4:
        {
        return addressOf(this->b_hit3_wav);
        break;
        }
    case 5:
        {
        return addressOf(this->b_hit2_wav);
        break;
        }
    case 6:
        {
        return addressOf(this->b_hit4_wav);
        break;
        }
    case 7:
        {
        return addressOf(this->b_hit5_wav);
        break;
        }
    case 8:
        {
        return addressOf(this->RNBODefaultMtofLookupTable256);
        break;
        }
    case 9:
        {
        return addressOf(this->RNBODefaultSinus);
        break;
        }
    default:
        {
        return nullptr;
        }
    }
}

DataRefIndex getNumDataRefs() const {
    return 10;
}

void fillRNBODefaultMtofLookupTable256(DataRef& ref) {
    Float64BufferRef buffer;
    buffer = new Float64Buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        number midivalue = -256. + (number)512. / (bufsize - 1) * i;
        buffer[i] = rnbo_exp(.057762265 * (midivalue - 69.0));
    }
}

void fillRNBODefaultSinus(DataRef& ref) {
    Float64BufferRef buffer;
    buffer = new Float64Buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        buffer[i] = rnbo_cos(i * 3.14159265358979323846 * 2. / bufsize);
    }
}

void fillDataRef(DataRefIndex index, DataRef& ref) {
    switch (index) {
    case 9:
        {
        this->fillRNBODefaultMtofLookupTable256(ref);
        break;
        }
    case 10:
        {
        this->fillRNBODefaultSinus(ref);
        break;
        }
    }
}

void zeroDataRef(DataRef& ref) {
    ref->setZero();
}

void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
    this->updateTime(time);

    if (index == 0) {
        this->data_01_buffer = new Float32Buffer(this->background);
        this->data_01_bufferUpdated();
        this->groove_01_buffer = new Float32Buffer(this->background);
    }

    if (index == 1) {
        this->data_02_buffer = new Float32Buffer(this->b_hit1_wav);
        this->data_02_bufferUpdated();
        this->multibuffertilde_01_buffer0 = new Float32Buffer(this->b_hit1_wav);
    }

    if (index == 2) {
        this->data_03_buffer = new Float32Buffer(this->bigbot);
        this->data_03_bufferUpdated();
        this->groove_04_buffer = new Float32Buffer(this->bigbot);
    }

    if (index == 3) {
        this->data_04_buffer = new Float32Buffer(this->littlebot);
        this->data_04_bufferUpdated();
        this->groove_02_buffer = new Float32Buffer(this->littlebot);
    }

    if (index == 4) {
        this->data_05_buffer = new Float32Buffer(this->b_hit3_wav);
        this->data_05_bufferUpdated();
        this->multibuffertilde_01_buffer1 = new Float32Buffer(this->b_hit3_wav);
    }

    if (index == 5) {
        this->data_06_buffer = new Float32Buffer(this->b_hit2_wav);
        this->data_06_bufferUpdated();
        this->multibuffertilde_01_buffer2 = new Float32Buffer(this->b_hit2_wav);
    }

    if (index == 6) {
        this->data_07_buffer = new Float32Buffer(this->b_hit4_wav);
        this->data_07_bufferUpdated();
        this->multibuffertilde_01_buffer3 = new Float32Buffer(this->b_hit4_wav);
    }

    if (index == 7) {
        this->data_08_buffer = new Float32Buffer(this->b_hit5_wav);
        this->data_08_bufferUpdated();
        this->multibuffertilde_01_buffer4 = new Float32Buffer(this->b_hit5_wav);
    }

    if (index == -1 || index == 1 || index == 4 || index == 5 || index == 6 || index == 7) {
        this->groove_03_buffer = new Float32MultiBuffer(this->drumchoice);
        this->multibuffertilde_01_buffer = new Float32MultiBuffer(this->drumchoice);
    }

    if (index == 8) {
        this->mtof_tilde_01_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
        this->mtof_tilde_02_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
        this->mtof_tilde_03_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
        this->mtof_tilde_04_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
    }

    if (index == 9) {
        this->cycle_tilde_01_buffer = new Float64Buffer(this->RNBODefaultSinus);
        this->cycle_tilde_01_bufferUpdated();
    }
}

void initialize() {
    this->background = initDataRef(
        "background",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/background.wav",
        "buffer~"
    );

    this->b_hit1_wav = initDataRef(
        "b_hit1_wav",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/hit1.wav",
        "buffer~"
    );

    this->bigbot = initDataRef(
        "bigbot",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/bigbot.wav",
        "buffer~"
    );

    this->littlebot = initDataRef(
        "littlebot",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/littlebot.wav",
        "buffer~"
    );

    this->b_hit3_wav = initDataRef(
        "b_hit3_wav",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/hit3.wav",
        "buffer~"
    );

    this->b_hit2_wav = initDataRef(
        "b_hit2_wav",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/hit2.wav",
        "buffer~"
    );

    this->b_hit4_wav = initDataRef(
        "b_hit4_wav",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/hit4.wav",
        "buffer~"
    );

    this->b_hit5_wav = initDataRef(
        "b_hit5_wav",
        false,
        "/Users/porridge/Games/Glitchblade/assets/sounds/hit5.wav",
        "buffer~"
    );

    this->drumchoice = initMultiRef(
        this->b_hit1_wav,
        this->b_hit3_wav,
        this->b_hit2_wav,
        this->b_hit4_wav,
        this->b_hit5_wav
    );

    this->RNBODefaultMtofLookupTable256 = initDataRef("RNBODefaultMtofLookupTable256", true, nullptr, "buffer~");
    this->RNBODefaultSinus = initDataRef("RNBODefaultSinus", true, nullptr, "buffer~");
    this->assign_defaults();
    this->setState();
    this->background->setIndex(0);
    this->data_01_buffer = new Float32Buffer(this->background);
    this->groove_01_buffer = new Float32Buffer(this->background);
    this->b_hit1_wav->setIndex(1);
    this->data_02_buffer = new Float32Buffer(this->b_hit1_wav);
    this->multibuffertilde_01_buffer0 = new Float32Buffer(this->b_hit1_wav);
    this->bigbot->setIndex(2);
    this->data_03_buffer = new Float32Buffer(this->bigbot);
    this->groove_04_buffer = new Float32Buffer(this->bigbot);
    this->littlebot->setIndex(3);
    this->data_04_buffer = new Float32Buffer(this->littlebot);
    this->groove_02_buffer = new Float32Buffer(this->littlebot);
    this->b_hit3_wav->setIndex(4);
    this->data_05_buffer = new Float32Buffer(this->b_hit3_wav);
    this->multibuffertilde_01_buffer1 = new Float32Buffer(this->b_hit3_wav);
    this->b_hit2_wav->setIndex(5);
    this->data_06_buffer = new Float32Buffer(this->b_hit2_wav);
    this->multibuffertilde_01_buffer2 = new Float32Buffer(this->b_hit2_wav);
    this->b_hit4_wav->setIndex(6);
    this->data_07_buffer = new Float32Buffer(this->b_hit4_wav);
    this->multibuffertilde_01_buffer3 = new Float32Buffer(this->b_hit4_wav);
    this->b_hit5_wav->setIndex(7);
    this->data_08_buffer = new Float32Buffer(this->b_hit5_wav);
    this->multibuffertilde_01_buffer4 = new Float32Buffer(this->b_hit5_wav);
    this->drumchoice->setIndex(-1);
    this->groove_03_buffer = new Float32MultiBuffer(this->drumchoice);
    this->multibuffertilde_01_buffer = new Float32MultiBuffer(this->drumchoice);
    this->RNBODefaultMtofLookupTable256->setIndex(8);
    this->mtof_tilde_01_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
    this->mtof_tilde_02_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
    this->mtof_tilde_03_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
    this->mtof_tilde_04_innerMtoF_buffer = new Float64Buffer(this->RNBODefaultMtofLookupTable256);
    this->RNBODefaultSinus->setIndex(9);
    this->cycle_tilde_01_buffer = new Float64Buffer(this->RNBODefaultSinus);
    this->initializeObjects();
    this->allocateDataRefs();
    this->startup();
}

Index getIsMuted()  {
    return this->isMuted;
}

void setIsMuted(Index v)  {
    this->isMuted = v;
}

void onSampleRateChanged(double samplerate) {
    this->timevalue_01_onSampleRateChanged(samplerate);
    this->timevalue_02_onSampleRateChanged(samplerate);
    this->timevalue_03_onSampleRateChanged(samplerate);
    this->timevalue_04_onSampleRateChanged(samplerate);
    this->timevalue_05_onSampleRateChanged(samplerate);
}

void processTempoEvent(MillisecondTime time, Tempo tempo) {
    this->updateTime(time);

    if (this->globaltransport_setTempo(this->_currentTime, tempo, false)) {
        this->transport_01_onTempoChanged(tempo);
        this->timevalue_01_onTempoChanged(tempo);
        this->timevalue_02_onTempoChanged(tempo);
        this->timevalue_03_onTempoChanged(tempo);
        this->timevalue_04_onTempoChanged(tempo);
        this->timevalue_05_onTempoChanged(tempo);
    }
}

void processTransportEvent(MillisecondTime time, TransportState state) {
    this->updateTime(time);

    if (this->globaltransport_setState(this->_currentTime, state, false)) {
        this->metro_01_onTransportChanged(state);
        this->transport_01_onTransportChanged(state);
        this->metro_02_onTransportChanged(state);
        this->metro_03_onTransportChanged(state);
        this->metro_04_onTransportChanged(state);
        this->metro_05_onTransportChanged(state);
    }
}

void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
    this->updateTime(time);

    if (this->globaltransport_setBeatTime(this->_currentTime, beattime, false)) {
        this->metro_01_onBeatTimeChanged(beattime);
        this->metro_02_onBeatTimeChanged(beattime);
        this->metro_03_onBeatTimeChanged(beattime);
        this->metro_04_onBeatTimeChanged(beattime);
        this->metro_05_onBeatTimeChanged(beattime);
    }
}

void processTimeSignatureEvent(MillisecondTime time, int numerator, int denominator) {
    this->updateTime(time);

    if (this->globaltransport_setTimeSignature(this->_currentTime, numerator, denominator, false)) {
        this->transport_01_onTimeSignatureChanged(numerator, denominator);
        this->timevalue_01_onTimeSignatureChanged(numerator, denominator);
        this->timevalue_02_onTimeSignatureChanged(numerator, denominator);
        this->timevalue_03_onTimeSignatureChanged(numerator, denominator);
        this->timevalue_04_onTimeSignatureChanged(numerator, denominator);
        this->timevalue_05_onTimeSignatureChanged(numerator, denominator);
    }
}

void processBBUEvent(MillisecondTime time, number bars, number beats, number units) {
    this->updateTime(time);

    if (this->globaltransport_setBBU(this->_currentTime, bars, beats, units, false))
        {}
}

Index getPatcherSerial() const {
    return 0;
}

void getState(PatcherStateInterface& ) {}

void setState() {}

void getPreset(PatcherStateInterface& preset) {
    preset["__presetid"] = "rnbo";
    this->param_01_getPresetValue(getSubState(preset, "startbang"));
    this->param_02_getPresetValue(getSubState(preset, "littlebang"));
    this->param_03_getPresetValue(getSubState(preset, "bigbang"));
    this->param_04_getPresetValue(getSubState(preset, "bigpan"));
}

void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
    this->updateTime(time);
    this->param_01_setPresetValue(getSubState(preset, "startbang"));
    this->param_02_setPresetValue(getSubState(preset, "littlebang"));
    this->param_03_setPresetValue(getSubState(preset, "bigbang"));
    this->param_04_setPresetValue(getSubState(preset, "bigpan"));
}

void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
    this->updateTime(time);

    switch (index) {
    case 0:
        {
        this->param_01_value_set(v);
        break;
        }
    case 1:
        {
        this->param_02_value_set(v);
        break;
        }
    case 2:
        {
        this->param_03_value_set(v);
        break;
        }
    case 3:
        {
        this->param_04_value_set(v);
        break;
        }
    }
}

void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValue(index, value, time);
}

void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
    this->setParameterValue(index, this->getParameterValue(index), time);
}

void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValueNormalized(index, value, time);
}

ParameterValue getParameterValue(ParameterIndex index)  {
    switch (index) {
    case 0:
        {
        return this->param_01_value;
        }
    case 1:
        {
        return this->param_02_value;
        }
    case 2:
        {
        return this->param_03_value;
        }
    case 3:
        {
        return this->param_04_value;
        }
    default:
        {
        return 0;
        }
    }
}

ParameterIndex getNumSignalInParameters() const {
    return 0;
}

ParameterIndex getNumSignalOutParameters() const {
    return 0;
}

ParameterIndex getNumParameters() const {
    return 4;
}

ConstCharPointer getParameterName(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "startbang";
        }
    case 1:
        {
        return "littlebang";
        }
    case 2:
        {
        return "bigbang";
        }
    case 3:
        {
        return "bigpan";
        }
    default:
        {
        return "bogus";
        }
    }
}

ConstCharPointer getParameterId(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "startbang";
        }
    case 1:
        {
        return "littlebang";
        }
    case 2:
        {
        return "bigbang";
        }
    case 3:
        {
        return "bigpan";
        }
    default:
        {
        return "bogus";
        }
    }
}

void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
    {
        switch (index) {
        case 0:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 1:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 2:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 3:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.5;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        }
    }
}

void sendParameter(ParameterIndex index, bool ignoreValue) {
    this->getEngine()->notifyParameterValueChanged(index, (ignoreValue ? 0 : this->getParameterValue(index)), ignoreValue);
}

ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
    if (steps == 1) {
        if (normalizedValue > 0) {
            normalizedValue = 1.;
        }
    } else {
        ParameterValue oneStep = (number)1. / (steps - 1);
        ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
        normalizedValue = numberOfSteps * oneStep;
    }

    return normalizedValue;
}

ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
    case 1:
    case 2:
    case 3:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            ParameterValue normalizedValue = (value - 0) / (1 - 0);
            return normalizedValue;
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    value = (value < 0 ? 0 : (value > 1 ? 1 : value));

    switch (index) {
    case 0:
    case 1:
    case 2:
    case 3:
        {
        {
            {
                return 0 + value * (1 - 0);
            }
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
        {
        return this->param_01_value_constrain(value);
        }
    case 1:
        {
        return this->param_02_value_constrain(value);
        }
    case 2:
        {
        return this->param_03_value_constrain(value);
        }
    case 3:
        {
        return this->param_04_value_constrain(value);
        }
    default:
        {
        return value;
        }
    }
}

void scheduleParamInit(ParameterIndex index, Index order) {
    this->paramInitIndices->push(index);
    this->paramInitOrder->push(order);
}

void processParamInitEvents() {
    this->listquicksort(
        this->paramInitOrder,
        this->paramInitIndices,
        0,
        (int)(this->paramInitOrder->length - 1),
        true
    );

    for (Index i = 0; i < this->paramInitOrder->length; i++) {
        this->getEngine()->scheduleParameterBang(this->paramInitIndices[i], 0);
    }
}

void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) {
    RNBO_UNUSED(hasValue);
    this->updateTime(time);

    switch (index) {
    case 1696335371:
        {
        this->loadbang_01_startupbang_bang();
        break;
        }
    case -1735137014:
        {
        this->loadbang_02_startupbang_bang();
        break;
        }
    case -871642103:
        {
        this->loadbang_03_startupbang_bang();
        break;
        }
    case 1673828112:
        {
        this->loadbang_04_startupbang_bang();
        break;
        }
    case -1214668403:
        {
        this->line_01_tick_set(value);
        break;
        }
    case -30654451:
        {
        this->loadbang_05_startupbang_bang();
        break;
        }
    case 1935387534:
        {
        this->metro_01_tick_bang();
        break;
        }
    case 1634454928:
        {
        this->line_02_tick_set(value);
        break;
        }
    case -1586675634:
        {
        this->line_03_tick_set(value);
        break;
        }
    case -1757644273:
        {
        this->loadbang_06_startupbang_bang();
        break;
        }
    case 1262447697:
        {
        this->line_04_tick_set(value);
        break;
        }
    case 890440466:
        {
        this->line_05_tick_set(value);
        break;
        }
    case 832840460:
        {
        this->loadbang_07_startupbang_bang();
        break;
        }
    case 518433235:
        {
        this->line_06_tick_set(value);
        break;
        }
    case -1172483444:
        {
        this->line_07_tick_set(value);
        break;
        }
    case 1592269969:
        {
        this->line_08_tick_set(value);
        break;
        }
    case -1958682865:
        {
        this->line_09_tick_set(value);
        break;
        }
    case -1544490675:
        {
        this->line_10_tick_set(value);
        break;
        }
    case 1220262738:
        {
        this->line_11_tick_set(value);
        break;
        }
    case -1916497906:
        {
        this->line_12_tick_set(value);
        break;
        }
    case 2006462159:
        {
        this->line_13_tick_set(value);
        break;
        }
    case -871550838:
        {
        this->metro_02_tick_bang();
        break;
        }
    case 1964277200:
        {
        this->line_14_tick_set(value);
        break;
        }
    case 146426004:
        {
        this->line_15_tick_set(value);
        break;
        }
    case 848255507:
        {
        this->line_16_tick_set(value);
        break;
        }
    case -894149362:
        {
        this->loadbang_08_startupbang_bang();
        break;
        }
    case -1357044121:
        {
        this->delay_01_out_bang();
        break;
        }
    case -1243558069:
        {
        this->metro_03_tick_bang();
        break;
        }
    case -225581227:
        {
        this->line_17_tick_set(value);
        break;
        }
    case -1615565300:
        {
        this->metro_04_tick_bang();
        break;
        }
    case -470653941:
        {
        this->line_18_tick_set(value);
        break;
        }
    case -1987572531:
        {
        this->metro_05_tick_bang();
        break;
        }
    case -842661172:
        {
        this->line_19_tick_set(value);
        break;
        }
    case -1468824490:
        {
        this->adsr_01_mute_bang();
        break;
        }
    }
}

void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}

void processOutletEvent(
    EngineLink* sender,
    OutletIndex index,
    ParameterValue value,
    MillisecondTime time
) {
    this->updateTime(time);
    this->processOutletAtCurrentTime(sender, index, value);
}

void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
    this->updateTime(time);

    switch (tag) {
    case TAG("valin"):
        {
        if (TAG("toggle_obj-103") == objectId)
            this->toggle_01_valin_set(payload);

        if (TAG("gain~_obj-122") == objectId)
            this->gaintilde_01_valin_set(payload);

        if (TAG("toggle_obj-96") == objectId)
            this->toggle_02_valin_set(payload);

        if (TAG("gain~_obj-124") == objectId)
            this->gaintilde_02_valin_set(payload);

        if (TAG("number_obj-214") == objectId)
            this->numberobj_01_valin_set(payload);

        if (TAG("toggle_obj-223") == objectId)
            this->toggle_03_valin_set(payload);

        if (TAG("number_obj-41") == objectId)
            this->numberobj_02_valin_set(payload);

        if (TAG("toggle_obj-73") == objectId)
            this->toggle_04_valin_set(payload);

        if (TAG("slider_obj-25") == objectId)
            this->slider_01_valin_set(payload);

        if (TAG("slider_obj-48") == objectId)
            this->slider_02_valin_set(payload);

        if (TAG("number_obj-30") == objectId)
            this->numberobj_03_valin_set(payload);

        if (TAG("gain~_obj-120") == objectId)
            this->gaintilde_03_valin_set(payload);

        if (TAG("number_obj-244") == objectId)
            this->numberobj_04_valin_set(payload);

        if (TAG("gain~_obj-116") == objectId)
            this->gaintilde_04_valin_set(payload);

        if (TAG("number_obj-212") == objectId)
            this->numberobj_05_valin_set(payload);

        if (TAG("slider_obj-32") == objectId)
            this->slider_03_valin_set(payload);

        if (TAG("slider_obj-49") == objectId)
            this->slider_04_valin_set(payload);

        if (TAG("number_obj-31") == objectId)
            this->numberobj_06_valin_set(payload);

        if (TAG("slider_obj-35") == objectId)
            this->slider_05_valin_set(payload);

        if (TAG("slider_obj-51") == objectId)
            this->slider_06_valin_set(payload);

        if (TAG("number_obj-34") == objectId)
            this->numberobj_07_valin_set(payload);

        if (TAG("number_obj-90") == objectId)
            this->numberobj_08_valin_set(payload);

        if (TAG("number_obj-253") == objectId)
            this->numberobj_09_valin_set(payload);

        if (TAG("slider_obj-38") == objectId)
            this->slider_07_valin_set(payload);

        if (TAG("slider_obj-53") == objectId)
            this->slider_08_valin_set(payload);

        if (TAG("toggle_obj-232") == objectId)
            this->toggle_05_valin_set(payload);

        if (TAG("gain~_obj-228") == objectId)
            this->gaintilde_05_valin_set(payload);

        if (TAG("number_obj-37") == objectId)
            this->numberobj_10_valin_set(payload);

        if (TAG("gain~_obj-227") == objectId)
            this->gaintilde_06_valin_set(payload);

        if (TAG("gain~_obj-18") == objectId)
            this->gaintilde_07_valin_set(payload);

        if (TAG("number_obj-82") == objectId)
            this->numberobj_11_valin_set(payload);

        if (TAG("toggle_obj-80") == objectId)
            this->toggle_06_valin_set(payload);

        if (TAG("gain~_obj-74") == objectId)
            this->gaintilde_08_valin_set(payload);

        if (TAG("number_obj-100") == objectId)
            this->numberobj_12_valin_set(payload);

        if (TAG("number_obj-262") == objectId)
            this->numberobj_13_valin_set(payload);

        if (TAG("number_obj-85") == objectId)
            this->numberobj_14_valin_set(payload);

        if (TAG("number_obj-94") == objectId)
            this->numberobj_15_valin_set(payload);

        if (TAG("number_obj-236") == objectId)
            this->numberobj_16_valin_set(payload);

        if (TAG("number_obj-121") == objectId)
            this->numberobj_17_valin_set(payload);

        if (TAG("number_obj-44") == objectId)
            this->numberobj_18_valin_set(payload);

        if (TAG("number_obj-151") == objectId)
            this->numberobj_19_valin_set(payload);

        if (TAG("toggle_obj-152") == objectId)
            this->toggle_07_valin_set(payload);

        if (TAG("number_obj-142") == objectId)
            this->numberobj_20_valin_set(payload);

        if (TAG("number_obj-149") == objectId)
            this->numberobj_21_valin_set(payload);

        if (TAG("number_obj-144") == objectId)
            this->numberobj_22_valin_set(payload);

        if (TAG("number_obj-137") == objectId)
            this->numberobj_23_valin_set(payload);

        if (TAG("number_obj-156") == objectId)
            this->numberobj_24_valin_set(payload);

        if (TAG("number_obj-175") == objectId)
            this->numberobj_25_valin_set(payload);

        if (TAG("toggle_obj-176") == objectId)
            this->toggle_08_valin_set(payload);

        if (TAG("number_obj-166") == objectId)
            this->numberobj_26_valin_set(payload);

        if (TAG("number_obj-173") == objectId)
            this->numberobj_27_valin_set(payload);

        if (TAG("number_obj-168") == objectId)
            this->numberobj_28_valin_set(payload);

        if (TAG("number_obj-161") == objectId)
            this->numberobj_29_valin_set(payload);

        if (TAG("number_obj-180") == objectId)
            this->numberobj_30_valin_set(payload);

        if (TAG("number_obj-199") == objectId)
            this->numberobj_31_valin_set(payload);

        if (TAG("toggle_obj-200") == objectId)
            this->toggle_09_valin_set(payload);

        if (TAG("number_obj-190") == objectId)
            this->numberobj_32_valin_set(payload);

        if (TAG("number_obj-197") == objectId)
            this->numberobj_33_valin_set(payload);

        if (TAG("number_obj-192") == objectId)
            this->numberobj_34_valin_set(payload);

        if (TAG("number_obj-185") == objectId)
            this->numberobj_35_valin_set(payload);

        if (TAG("number_obj-204") == objectId)
            this->numberobj_36_valin_set(payload);

        break;
        }
    case TAG("listin"):
        {
        if (TAG("message_obj-126") == objectId)
            this->message_01_listin_number_set(payload);

        if (TAG("message_obj-216") == objectId)
            this->message_02_listin_number_set(payload);

        if (TAG("message_obj-56") == objectId)
            this->message_03_listin_number_set(payload);

        if (TAG("message_obj-5") == objectId)
            this->message_04_listin_number_set(payload);

        if (TAG("message_obj-75") == objectId)
            this->message_05_listin_number_set(payload);

        if (TAG("message_obj-10") == objectId)
            this->message_06_listin_number_set(payload);

        if (TAG("message_obj-243") == objectId)
            this->message_07_listin_number_set(payload);

        if (TAG("message_obj-12") == objectId)
            this->message_08_listin_number_set(payload);

        if (TAG("message_obj-17") == objectId)
            this->message_09_listin_number_set(payload);

        if (TAG("message_obj-59") == objectId)
            this->message_10_listin_number_set(payload);

        if (TAG("message_obj-129") == objectId)
            this->message_11_listin_number_set(payload);

        if (TAG("message_obj-60") == objectId)
            this->message_12_listin_number_set(payload);

        if (TAG("message_obj-135") == objectId)
            this->message_13_listin_number_set(payload);

        if (TAG("message_obj-252") == objectId)
            this->message_14_listin_number_set(payload);

        if (TAG("message_obj-62") == objectId)
            this->message_15_listin_number_set(payload);

        if (TAG("message_obj-2") == objectId)
            this->message_16_listin_number_set(payload);

        if (TAG("message_obj-64") == objectId)
            this->message_17_listin_number_set(payload);

        if (TAG("message_obj-211") == objectId)
            this->message_18_listin_number_set(payload);

        if (TAG("message_obj-261") == objectId)
            this->message_19_listin_number_set(payload);

        if (TAG("message_obj-219") == objectId)
            this->message_20_listin_number_set(payload);

        if (TAG("message_obj-106") == objectId)
            this->message_21_listin_number_set(payload);

        if (TAG("message_obj-141") == objectId)
            this->message_22_listin_number_set(payload);

        if (TAG("message_obj-165") == objectId)
            this->message_23_listin_number_set(payload);

        if (TAG("message_obj-189") == objectId)
            this->message_24_listin_number_set(payload);

        break;
        }
    case TAG("format"):
        {
        if (TAG("number_obj-214") == objectId)
            this->numberobj_01_format_set(payload);

        if (TAG("number_obj-41") == objectId)
            this->numberobj_02_format_set(payload);

        if (TAG("number_obj-30") == objectId)
            this->numberobj_03_format_set(payload);

        if (TAG("number_obj-244") == objectId)
            this->numberobj_04_format_set(payload);

        if (TAG("number_obj-212") == objectId)
            this->numberobj_05_format_set(payload);

        if (TAG("number_obj-31") == objectId)
            this->numberobj_06_format_set(payload);

        if (TAG("number_obj-34") == objectId)
            this->numberobj_07_format_set(payload);

        if (TAG("number_obj-90") == objectId)
            this->numberobj_08_format_set(payload);

        if (TAG("number_obj-253") == objectId)
            this->numberobj_09_format_set(payload);

        if (TAG("number_obj-37") == objectId)
            this->numberobj_10_format_set(payload);

        if (TAG("number_obj-82") == objectId)
            this->numberobj_11_format_set(payload);

        if (TAG("number_obj-100") == objectId)
            this->numberobj_12_format_set(payload);

        if (TAG("number_obj-262") == objectId)
            this->numberobj_13_format_set(payload);

        if (TAG("number_obj-85") == objectId)
            this->numberobj_14_format_set(payload);

        if (TAG("number_obj-94") == objectId)
            this->numberobj_15_format_set(payload);

        if (TAG("number_obj-236") == objectId)
            this->numberobj_16_format_set(payload);

        if (TAG("number_obj-121") == objectId)
            this->numberobj_17_format_set(payload);

        if (TAG("number_obj-44") == objectId)
            this->numberobj_18_format_set(payload);

        if (TAG("number_obj-151") == objectId)
            this->numberobj_19_format_set(payload);

        if (TAG("number_obj-142") == objectId)
            this->numberobj_20_format_set(payload);

        if (TAG("number_obj-149") == objectId)
            this->numberobj_21_format_set(payload);

        if (TAG("number_obj-144") == objectId)
            this->numberobj_22_format_set(payload);

        if (TAG("number_obj-137") == objectId)
            this->numberobj_23_format_set(payload);

        if (TAG("number_obj-156") == objectId)
            this->numberobj_24_format_set(payload);

        if (TAG("number_obj-175") == objectId)
            this->numberobj_25_format_set(payload);

        if (TAG("number_obj-166") == objectId)
            this->numberobj_26_format_set(payload);

        if (TAG("number_obj-173") == objectId)
            this->numberobj_27_format_set(payload);

        if (TAG("number_obj-168") == objectId)
            this->numberobj_28_format_set(payload);

        if (TAG("number_obj-161") == objectId)
            this->numberobj_29_format_set(payload);

        if (TAG("number_obj-180") == objectId)
            this->numberobj_30_format_set(payload);

        if (TAG("number_obj-199") == objectId)
            this->numberobj_31_format_set(payload);

        if (TAG("number_obj-190") == objectId)
            this->numberobj_32_format_set(payload);

        if (TAG("number_obj-197") == objectId)
            this->numberobj_33_format_set(payload);

        if (TAG("number_obj-192") == objectId)
            this->numberobj_34_format_set(payload);

        if (TAG("number_obj-185") == objectId)
            this->numberobj_35_format_set(payload);

        if (TAG("number_obj-204") == objectId)
            this->numberobj_36_format_set(payload);

        break;
        }
    }
}

void processListMessage(
    MessageTag tag,
    MessageTag objectId,
    MillisecondTime time,
    const list& payload
) {
    this->updateTime(time);

    switch (tag) {
    case TAG("listin"):
        {
        if (TAG("message_obj-126") == objectId)
            this->message_01_listin_list_set(payload);

        if (TAG("message_obj-216") == objectId)
            this->message_02_listin_list_set(payload);

        if (TAG("message_obj-56") == objectId)
            this->message_03_listin_list_set(payload);

        if (TAG("message_obj-5") == objectId)
            this->message_04_listin_list_set(payload);

        if (TAG("message_obj-75") == objectId)
            this->message_05_listin_list_set(payload);

        if (TAG("message_obj-10") == objectId)
            this->message_06_listin_list_set(payload);

        if (TAG("message_obj-243") == objectId)
            this->message_07_listin_list_set(payload);

        if (TAG("message_obj-12") == objectId)
            this->message_08_listin_list_set(payload);

        if (TAG("message_obj-17") == objectId)
            this->message_09_listin_list_set(payload);

        if (TAG("message_obj-59") == objectId)
            this->message_10_listin_list_set(payload);

        if (TAG("message_obj-129") == objectId)
            this->message_11_listin_list_set(payload);

        if (TAG("message_obj-60") == objectId)
            this->message_12_listin_list_set(payload);

        if (TAG("message_obj-135") == objectId)
            this->message_13_listin_list_set(payload);

        if (TAG("message_obj-252") == objectId)
            this->message_14_listin_list_set(payload);

        if (TAG("message_obj-62") == objectId)
            this->message_15_listin_list_set(payload);

        if (TAG("message_obj-2") == objectId)
            this->message_16_listin_list_set(payload);

        if (TAG("message_obj-64") == objectId)
            this->message_17_listin_list_set(payload);

        if (TAG("message_obj-211") == objectId)
            this->message_18_listin_list_set(payload);

        if (TAG("message_obj-261") == objectId)
            this->message_19_listin_list_set(payload);

        if (TAG("message_obj-219") == objectId)
            this->message_20_listin_list_set(payload);

        if (TAG("message_obj-106") == objectId)
            this->message_21_listin_list_set(payload);

        if (TAG("message_obj-141") == objectId)
            this->message_22_listin_list_set(payload);

        if (TAG("message_obj-165") == objectId)
            this->message_23_listin_list_set(payload);

        if (TAG("message_obj-189") == objectId)
            this->message_24_listin_list_set(payload);

        break;
        }
    }
}

void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
    this->updateTime(time);

    switch (tag) {
    case TAG("bangin"):
        {
        if (TAG("button_obj-8") == objectId)
            this->button_01_bangin_bang();

        if (TAG("button_obj-57") == objectId)
            this->button_02_bangin_bang();

        if (TAG("button_obj-233") == objectId)
            this->button_03_bangin_bang();

        if (TAG("button_obj-117") == objectId)
            this->button_04_bangin_bang();

        if (TAG("button_obj-218") == objectId)
            this->button_05_bangin_bang();

        if (TAG("button_obj-240") == objectId)
            this->button_06_bangin_bang();

        if (TAG("button_obj-217") == objectId)
            this->button_07_bangin_bang();

        if (TAG("button_obj-242") == objectId)
            this->button_08_bangin_bang();

        if (TAG("button_obj-84") == objectId)
            this->button_09_bangin_bang();

        if (TAG("button_obj-249") == objectId)
            this->button_10_bangin_bang();

        if (TAG("button_obj-251") == objectId)
            this->button_11_bangin_bang();

        if (TAG("button_obj-7") == objectId)
            this->button_12_bangin_bang();

        if (TAG("button_obj-258") == objectId)
            this->button_13_bangin_bang();

        if (TAG("button_obj-78") == objectId)
            this->button_14_bangin_bang();

        if (TAG("button_obj-260") == objectId)
            this->button_15_bangin_bang();

        if (TAG("button_obj-133") == objectId)
            this->button_16_bangin_bang();

        if (TAG("button_obj-98") == objectId)
            this->button_17_bangin_bang();

        if (TAG("button_obj-72") == objectId)
            this->button_18_bangin_bang();

        if (TAG("button_obj-153") == objectId)
            this->button_19_bangin_bang();

        if (TAG("button_obj-134") == objectId)
            this->button_20_bangin_bang();

        if (TAG("button_obj-143") == objectId)
            this->button_21_bangin_bang();

        if (TAG("button_obj-177") == objectId)
            this->button_22_bangin_bang();

        if (TAG("button_obj-158") == objectId)
            this->button_23_bangin_bang();

        if (TAG("button_obj-167") == objectId)
            this->button_24_bangin_bang();

        if (TAG("button_obj-201") == objectId)
            this->button_25_bangin_bang();

        if (TAG("button_obj-182") == objectId)
            this->button_26_bangin_bang();

        if (TAG("button_obj-191") == objectId)
            this->button_27_bangin_bang();

        break;
        }
    case TAG("startupbang"):
        {
        if (TAG("loadbang_obj-104") == objectId)
            this->loadbang_01_startupbang_bang();

        if (TAG("loadbang_obj-127") == objectId)
            this->loadbang_02_startupbang_bang();

        if (TAG("loadbang_obj-45") == objectId)
            this->loadbang_03_startupbang_bang();

        if (TAG("loadbang_obj-291") == objectId)
            this->loadbang_04_startupbang_bang();

        if (TAG("loadbang_obj-224") == objectId)
            this->loadbang_05_startupbang_bang();

        if (TAG("loadbang_obj-3") == objectId)
            this->loadbang_06_startupbang_bang();

        if (TAG("loadbang_obj-13") == objectId)
            this->loadbang_07_startupbang_bang();

        if (TAG("loadbang_obj-4") == objectId)
            this->loadbang_08_startupbang_bang();

        break;
        }
    case TAG("listin"):
        {
        if (TAG("message_obj-126") == objectId)
            this->message_01_listin_bang_bang();

        if (TAG("message_obj-216") == objectId)
            this->message_02_listin_bang_bang();

        if (TAG("message_obj-56") == objectId)
            this->message_03_listin_bang_bang();

        if (TAG("message_obj-5") == objectId)
            this->message_04_listin_bang_bang();

        if (TAG("message_obj-75") == objectId)
            this->message_05_listin_bang_bang();

        if (TAG("message_obj-10") == objectId)
            this->message_06_listin_bang_bang();

        if (TAG("message_obj-243") == objectId)
            this->message_07_listin_bang_bang();

        if (TAG("message_obj-12") == objectId)
            this->message_08_listin_bang_bang();

        if (TAG("message_obj-17") == objectId)
            this->message_09_listin_bang_bang();

        if (TAG("message_obj-59") == objectId)
            this->message_10_listin_bang_bang();

        if (TAG("message_obj-129") == objectId)
            this->message_11_listin_bang_bang();

        if (TAG("message_obj-60") == objectId)
            this->message_12_listin_bang_bang();

        if (TAG("message_obj-135") == objectId)
            this->message_13_listin_bang_bang();

        if (TAG("message_obj-252") == objectId)
            this->message_14_listin_bang_bang();

        if (TAG("message_obj-62") == objectId)
            this->message_15_listin_bang_bang();

        if (TAG("message_obj-2") == objectId)
            this->message_16_listin_bang_bang();

        if (TAG("message_obj-64") == objectId)
            this->message_17_listin_bang_bang();

        if (TAG("message_obj-211") == objectId)
            this->message_18_listin_bang_bang();

        if (TAG("message_obj-261") == objectId)
            this->message_19_listin_bang_bang();

        if (TAG("message_obj-219") == objectId)
            this->message_20_listin_bang_bang();

        if (TAG("message_obj-106") == objectId)
            this->message_21_listin_bang_bang();

        if (TAG("message_obj-141") == objectId)
            this->message_22_listin_bang_bang();

        if (TAG("message_obj-165") == objectId)
            this->message_23_listin_bang_bang();

        if (TAG("message_obj-189") == objectId)
            this->message_24_listin_bang_bang();

        break;
        }
    }
}

MessageTagInfo resolveTag(MessageTag tag) const {
    switch (tag) {
    case TAG("bangout"):
        {
        return "bangout";
        }
    case TAG("button_obj-8"):
        {
        return "button_obj-8";
        }
    case TAG("valout"):
        {
        return "valout";
        }
    case TAG("toggle_obj-103"):
        {
        return "toggle_obj-103";
        }
    case TAG("listout"):
        {
        return "listout";
        }
    case TAG("message_obj-126"):
        {
        return "message_obj-126";
        }
    case TAG("button_obj-57"):
        {
        return "button_obj-57";
        }
    case TAG("gain~_obj-122"):
        {
        return "gain~_obj-122";
        }
    case TAG("toggle_obj-96"):
        {
        return "toggle_obj-96";
        }
    case TAG("button_obj-233"):
        {
        return "button_obj-233";
        }
    case TAG("gain~_obj-124"):
        {
        return "gain~_obj-124";
        }
    case TAG("number_obj-214"):
        {
        return "number_obj-214";
        }
    case TAG("setup"):
        {
        return "setup";
        }
    case TAG("button_obj-117"):
        {
        return "button_obj-117";
        }
    case TAG("button_obj-218"):
        {
        return "button_obj-218";
        }
    case TAG("message_obj-216"):
        {
        return "message_obj-216";
        }
    case TAG("message_obj-56"):
        {
        return "message_obj-56";
        }
    case TAG("toggle_obj-223"):
        {
        return "toggle_obj-223";
        }
    case TAG("number_obj-41"):
        {
        return "number_obj-41";
        }
    case TAG("toggle_obj-73"):
        {
        return "toggle_obj-73";
        }
    case TAG("slider_obj-25"):
        {
        return "slider_obj-25";
        }
    case TAG("message_obj-5"):
        {
        return "message_obj-5";
        }
    case TAG("slider_obj-48"):
        {
        return "slider_obj-48";
        }
    case TAG("number_obj-30"):
        {
        return "number_obj-30";
        }
    case TAG("gain~_obj-120"):
        {
        return "gain~_obj-120";
        }
    case TAG("button_obj-240"):
        {
        return "button_obj-240";
        }
    case TAG("message_obj-75"):
        {
        return "message_obj-75";
        }
    case TAG("message_obj-10"):
        {
        return "message_obj-10";
        }
    case TAG("number_obj-244"):
        {
        return "number_obj-244";
        }
    case TAG("gain~_obj-116"):
        {
        return "gain~_obj-116";
        }
    case TAG("number_obj-212"):
        {
        return "number_obj-212";
        }
    case TAG("button_obj-217"):
        {
        return "button_obj-217";
        }
    case TAG("slider_obj-32"):
        {
        return "slider_obj-32";
        }
    case TAG("button_obj-242"):
        {
        return "button_obj-242";
        }
    case TAG("message_obj-243"):
        {
        return "message_obj-243";
        }
    case TAG("slider_obj-49"):
        {
        return "slider_obj-49";
        }
    case TAG("message_obj-12"):
        {
        return "message_obj-12";
        }
    case TAG("number_obj-31"):
        {
        return "number_obj-31";
        }
    case TAG("message_obj-17"):
        {
        return "message_obj-17";
        }
    case TAG("slider_obj-35"):
        {
        return "slider_obj-35";
        }
    case TAG("slider_obj-51"):
        {
        return "slider_obj-51";
        }
    case TAG("message_obj-59"):
        {
        return "message_obj-59";
        }
    case TAG("message_obj-129"):
        {
        return "message_obj-129";
        }
    case TAG("button_obj-84"):
        {
        return "button_obj-84";
        }
    case TAG("number_obj-34"):
        {
        return "number_obj-34";
        }
    case TAG("button_obj-249"):
        {
        return "button_obj-249";
        }
    case TAG("message_obj-60"):
        {
        return "message_obj-60";
        }
    case TAG("number_obj-90"):
        {
        return "number_obj-90";
        }
    case TAG("number_obj-253"):
        {
        return "number_obj-253";
        }
    case TAG("slider_obj-38"):
        {
        return "slider_obj-38";
        }
    case TAG("button_obj-251"):
        {
        return "button_obj-251";
        }
    case TAG("message_obj-135"):
        {
        return "message_obj-135";
        }
    case TAG("message_obj-252"):
        {
        return "message_obj-252";
        }
    case TAG("slider_obj-53"):
        {
        return "slider_obj-53";
        }
    case TAG("toggle_obj-232"):
        {
        return "toggle_obj-232";
        }
    case TAG("gain~_obj-228"):
        {
        return "gain~_obj-228";
        }
    case TAG("message_obj-62"):
        {
        return "message_obj-62";
        }
    case TAG("number_obj-37"):
        {
        return "number_obj-37";
        }
    case TAG("message_obj-2"):
        {
        return "message_obj-2";
        }
    case TAG("gain~_obj-227"):
        {
        return "gain~_obj-227";
        }
    case TAG("message_obj-64"):
        {
        return "message_obj-64";
        }
    case TAG("gain~_obj-18"):
        {
        return "gain~_obj-18";
        }
    case TAG("button_obj-7"):
        {
        return "button_obj-7";
        }
    case TAG("number_obj-82"):
        {
        return "number_obj-82";
        }
    case TAG("toggle_obj-80"):
        {
        return "toggle_obj-80";
        }
    case TAG("button_obj-258"):
        {
        return "button_obj-258";
        }
    case TAG("gain~_obj-74"):
        {
        return "gain~_obj-74";
        }
    case TAG("button_obj-78"):
        {
        return "button_obj-78";
        }
    case TAG("message_obj-211"):
        {
        return "message_obj-211";
        }
    case TAG("number_obj-100"):
        {
        return "number_obj-100";
        }
    case TAG("number_obj-262"):
        {
        return "number_obj-262";
        }
    case TAG("number_obj-85"):
        {
        return "number_obj-85";
        }
    case TAG("button_obj-260"):
        {
        return "button_obj-260";
        }
    case TAG("message_obj-261"):
        {
        return "message_obj-261";
        }
    case TAG("number_obj-94"):
        {
        return "number_obj-94";
        }
    case TAG("button_obj-133"):
        {
        return "button_obj-133";
        }
    case TAG("button_obj-98"):
        {
        return "button_obj-98";
        }
    case TAG("number_obj-236"):
        {
        return "number_obj-236";
        }
    case TAG("message_obj-219"):
        {
        return "message_obj-219";
        }
    case TAG("number_obj-121"):
        {
        return "number_obj-121";
        }
    case TAG("message_obj-106"):
        {
        return "message_obj-106";
        }
    case TAG("number_obj-44"):
        {
        return "number_obj-44";
        }
    case TAG("button_obj-72"):
        {
        return "button_obj-72";
        }
    case TAG("number_obj-151"):
        {
        return "number_obj-151";
        }
    case TAG("toggle_obj-152"):
        {
        return "toggle_obj-152";
        }
    case TAG("button_obj-153"):
        {
        return "button_obj-153";
        }
    case TAG("number_obj-142"):
        {
        return "number_obj-142";
        }
    case TAG("number_obj-149"):
        {
        return "number_obj-149";
        }
    case TAG("number_obj-144"):
        {
        return "number_obj-144";
        }
    case TAG("button_obj-134"):
        {
        return "button_obj-134";
        }
    case TAG("button_obj-143"):
        {
        return "button_obj-143";
        }
    case TAG("number_obj-137"):
        {
        return "number_obj-137";
        }
    case TAG("message_obj-141"):
        {
        return "message_obj-141";
        }
    case TAG("number_obj-156"):
        {
        return "number_obj-156";
        }
    case TAG("number_obj-175"):
        {
        return "number_obj-175";
        }
    case TAG("toggle_obj-176"):
        {
        return "toggle_obj-176";
        }
    case TAG("button_obj-177"):
        {
        return "button_obj-177";
        }
    case TAG("number_obj-166"):
        {
        return "number_obj-166";
        }
    case TAG("number_obj-173"):
        {
        return "number_obj-173";
        }
    case TAG("number_obj-168"):
        {
        return "number_obj-168";
        }
    case TAG("button_obj-158"):
        {
        return "button_obj-158";
        }
    case TAG("button_obj-167"):
        {
        return "button_obj-167";
        }
    case TAG("number_obj-161"):
        {
        return "number_obj-161";
        }
    case TAG("message_obj-165"):
        {
        return "message_obj-165";
        }
    case TAG("number_obj-180"):
        {
        return "number_obj-180";
        }
    case TAG("number_obj-199"):
        {
        return "number_obj-199";
        }
    case TAG("toggle_obj-200"):
        {
        return "toggle_obj-200";
        }
    case TAG("button_obj-201"):
        {
        return "button_obj-201";
        }
    case TAG("number_obj-190"):
        {
        return "number_obj-190";
        }
    case TAG("number_obj-197"):
        {
        return "number_obj-197";
        }
    case TAG("number_obj-192"):
        {
        return "number_obj-192";
        }
    case TAG("button_obj-182"):
        {
        return "button_obj-182";
        }
    case TAG("button_obj-191"):
        {
        return "button_obj-191";
        }
    case TAG("number_obj-185"):
        {
        return "number_obj-185";
        }
    case TAG("message_obj-189"):
        {
        return "message_obj-189";
        }
    case TAG("number_obj-204"):
        {
        return "number_obj-204";
        }
    case TAG("bangin"):
        {
        return "bangin";
        }
    case TAG("startupbang"):
        {
        return "startupbang";
        }
    case TAG("loadbang_obj-104"):
        {
        return "loadbang_obj-104";
        }
    case TAG("valin"):
        {
        return "valin";
        }
    case TAG("loadbang_obj-127"):
        {
        return "loadbang_obj-127";
        }
    case TAG("listin"):
        {
        return "listin";
        }
    case TAG("loadbang_obj-45"):
        {
        return "loadbang_obj-45";
        }
    case TAG("loadbang_obj-291"):
        {
        return "loadbang_obj-291";
        }
    case TAG("format"):
        {
        return "format";
        }
    case TAG("loadbang_obj-224"):
        {
        return "loadbang_obj-224";
        }
    case TAG("loadbang_obj-3"):
        {
        return "loadbang_obj-3";
        }
    case TAG("loadbang_obj-13"):
        {
        return "loadbang_obj-13";
        }
    case TAG("loadbang_obj-4"):
        {
        return "loadbang_obj-4";
        }
    }

    return "";
}

MessageIndex getNumMessages() const {
    return 0;
}

const MessageInfo& getMessageInfo(MessageIndex index) const {
    switch (index) {

    }

    return NullMessageInfo;
}

protected:

void param_01_value_set(number v) {
    v = this->param_01_value_constrain(v);
    this->param_01_value = v;
    this->sendParameter(0, false);

    if (this->param_01_value != this->param_01_lastValue) {
        this->getEngine()->presetTouched();
        this->param_01_lastValue = this->param_01_value;
    }

    this->button_04_bangval_bang();
    this->button_02_bangval_bang();
}

void param_02_value_set(number v) {
    v = this->param_02_value_constrain(v);
    this->param_02_value = v;
    this->sendParameter(1, false);

    if (this->param_02_value != this->param_02_lastValue) {
        this->getEngine()->presetTouched();
        this->param_02_lastValue = this->param_02_value;
    }

    this->select_02_input_number_set(v);
}

void param_03_value_set(number v) {
    v = this->param_03_value_constrain(v);
    this->param_03_value = v;
    this->sendParameter(2, false);

    if (this->param_03_value != this->param_03_lastValue) {
        this->getEngine()->presetTouched();
        this->param_03_lastValue = this->param_03_value;
    }

    this->select_03_input_number_set(v);
}

void param_04_value_set(number v) {
    v = this->param_04_value_constrain(v);
    this->param_04_value = v;
    this->sendParameter(3, false);

    if (this->param_04_value != this->param_04_lastValue) {
        this->getEngine()->presetTouched();
        this->param_04_lastValue = this->param_04_value;
    }

    this->numberobj_16_value_set(v);
    this->pan_tilde_05_pos_set(v);
}

void button_01_bangin_bang() {
    this->button_01_bangval_bang();
}

void loadbang_01_startupbang_bang() {
    this->loadbang_01_output_bang();
}

void toggle_01_valin_set(number v) {
    this->toggle_01_value_number_set(v);
}

void loadbang_02_startupbang_bang() {
    this->loadbang_02_output_bang();
}

void message_01_listin_list_set(const list& v) {
    this->message_01_set_set(v);
}

void message_01_listin_number_set(number v) {
    this->message_01_set_set(v);
}

void message_01_listin_bang_bang() {
    this->message_01_trigger_bang();
}

void loadbang_03_startupbang_bang() {
    this->loadbang_03_output_bang();
}

void button_02_bangin_bang() {
    this->button_02_bangval_bang();
}

void gaintilde_01_valin_set(number v) {
    this->gaintilde_01_value_set(v);
}

void toggle_02_valin_set(number v) {
    this->toggle_02_value_number_set(v);
}

void loadbang_04_startupbang_bang() {
    this->loadbang_04_output_bang();
}

void button_03_bangin_bang() {
    this->button_03_bangval_bang();
}

void line_01_tick_set(number v) {
    this->line_01_output_set(v);

    if ((bool)(this->line_01_isFinished(v))) {
        this->line_01_slope = 0;
        this->line_01_startValue = v;
        this->line_01_startPendingRamp();
    } else {
        this->line_01_scheduleNext();
    }
}

void gaintilde_02_valin_set(number v) {
    this->gaintilde_02_value_set(v);
}

void numberobj_01_valin_set(number v) {
    this->numberobj_01_value_set(v);
}

void numberobj_01_format_set(number v) {
    this->numberobj_01_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_04_bangin_bang() {
    this->button_04_bangval_bang();
}

void button_05_bangin_bang() {
    this->button_05_bangval_bang();
}

void message_02_listin_list_set(const list& v) {
    this->message_02_set_set(v);
}

void message_02_listin_number_set(number v) {
    this->message_02_set_set(v);
}

void message_02_listin_bang_bang() {
    this->message_02_trigger_bang();
}

void message_03_listin_list_set(const list& v) {
    this->message_03_set_set(v);
}

void message_03_listin_number_set(number v) {
    this->message_03_set_set(v);
}

void message_03_listin_bang_bang() {
    this->message_03_trigger_bang();
}

void loadbang_05_startupbang_bang() {
    this->loadbang_05_output_bang();
}

void toggle_03_valin_set(number v) {
    this->toggle_03_value_number_set(v);
}

void numberobj_02_valin_set(number v) {
    this->numberobj_02_value_set(v);
}

void numberobj_02_format_set(number v) {
    this->numberobj_02_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void metro_01_tick_bang() {
    this->metro_01_tickout_bang();
    this->getEngine()->flushClockEvents(this, 1935387534, false);;

    if ((bool)(this->metro_01_on)) {
        this->metro_01_last = this->currenttime();

        {
            this->metro_01_next = this->metro_01_last + this->metro_01_interval;
            this->getEngine()->scheduleClockEvent(this, 1935387534, this->metro_01_interval + this->_currentTime);;
        }
    }
}

void toggle_04_valin_set(number v) {
    this->toggle_04_value_number_set(v);
}

void slider_01_valin_set(number v) {
    this->slider_01_value_set(v);
}

void line_02_tick_set(number v) {
    this->line_02_output_set(v);

    if ((bool)(this->line_02_isFinished(v))) {
        this->line_02_slope = 0;
        this->line_02_startValue = v;
        this->line_02_startPendingRamp();
    } else {
        this->line_02_scheduleNext();
    }
}

void message_04_listin_list_set(const list& v) {
    this->message_04_set_set(v);
}

void message_04_listin_number_set(number v) {
    this->message_04_set_set(v);
}

void message_04_listin_bang_bang() {
    this->message_04_trigger_bang();
}

void slider_02_valin_set(number v) {
    this->slider_02_value_set(v);
}

void numberobj_03_valin_set(number v) {
    this->numberobj_03_value_set(v);
}

void numberobj_03_format_set(number v) {
    this->numberobj_03_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void gaintilde_03_valin_set(number v) {
    this->gaintilde_03_value_set(v);
}

void button_06_bangin_bang() {
    this->button_06_bangval_bang();
}

void line_03_tick_set(number v) {
    this->line_03_output_set(v);

    if ((bool)(this->line_03_isFinished(v))) {
        this->line_03_slope = 0;
        this->line_03_startValue = v;
        this->line_03_startPendingRamp();
    } else {
        this->line_03_scheduleNext();
    }
}

void loadbang_06_startupbang_bang() {
    this->loadbang_06_output_bang();
}

void message_05_listin_list_set(const list& v) {
    this->message_05_set_set(v);
}

void message_05_listin_number_set(number v) {
    this->message_05_set_set(v);
}

void message_05_listin_bang_bang() {
    this->message_05_trigger_bang();
}

void line_04_tick_set(number v) {
    this->line_04_output_set(v);

    if ((bool)(this->line_04_isFinished(v))) {
        this->line_04_slope = 0;
        this->line_04_startValue = v;
        this->line_04_startPendingRamp();
    } else {
        this->line_04_scheduleNext();
    }
}

void message_06_listin_list_set(const list& v) {
    this->message_06_set_set(v);
}

void message_06_listin_number_set(number v) {
    this->message_06_set_set(v);
}

void message_06_listin_bang_bang() {
    this->message_06_trigger_bang();
}

void numberobj_04_valin_set(number v) {
    this->numberobj_04_value_set(v);
}

void numberobj_04_format_set(number v) {
    this->numberobj_04_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void gaintilde_04_valin_set(number v) {
    this->gaintilde_04_value_set(v);
}

void numberobj_05_valin_set(number v) {
    this->numberobj_05_value_set(v);
}

void numberobj_05_format_set(number v) {
    this->numberobj_05_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_07_bangin_bang() {
    this->button_07_bangval_bang();
}

void slider_03_valin_set(number v) {
    this->slider_03_value_set(v);
}

void button_08_bangin_bang() {
    this->button_08_bangval_bang();
}

void message_07_listin_list_set(const list& v) {
    this->message_07_set_set(v);
}

void message_07_listin_number_set(number v) {
    this->message_07_set_set(v);
}

void message_07_listin_bang_bang() {
    this->message_07_trigger_bang();
}

void slider_04_valin_set(number v) {
    this->slider_04_value_set(v);
}

void line_05_tick_set(number v) {
    this->line_05_output_set(v);

    if ((bool)(this->line_05_isFinished(v))) {
        this->line_05_slope = 0;
        this->line_05_startValue = v;
        this->line_05_startPendingRamp();
    } else {
        this->line_05_scheduleNext();
    }
}

void message_08_listin_list_set(const list& v) {
    this->message_08_set_set(v);
}

void message_08_listin_number_set(number v) {
    this->message_08_set_set(v);
}

void message_08_listin_bang_bang() {
    this->message_08_trigger_bang();
}

void numberobj_06_valin_set(number v) {
    this->numberobj_06_value_set(v);
}

void numberobj_06_format_set(number v) {
    this->numberobj_06_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void loadbang_07_startupbang_bang() {
    this->loadbang_07_output_bang();
}

void line_06_tick_set(number v) {
    this->line_06_output_set(v);

    if ((bool)(this->line_06_isFinished(v))) {
        this->line_06_slope = 0;
        this->line_06_startValue = v;
        this->line_06_startPendingRamp();
    } else {
        this->line_06_scheduleNext();
    }
}

void message_09_listin_list_set(const list& v) {
    this->message_09_set_set(v);
}

void message_09_listin_number_set(number v) {
    this->message_09_set_set(v);
}

void message_09_listin_bang_bang() {
    this->message_09_trigger_bang();
}

void slider_05_valin_set(number v) {
    this->slider_05_value_set(v);
}

void slider_06_valin_set(number v) {
    this->slider_06_value_set(v);
}

void message_10_listin_list_set(const list& v) {
    this->message_10_set_set(v);
}

void message_10_listin_number_set(number v) {
    this->message_10_set_set(v);
}

void message_10_listin_bang_bang() {
    this->message_10_trigger_bang();
}

void line_07_tick_set(number v) {
    this->line_07_output_set(v);

    if ((bool)(this->line_07_isFinished(v))) {
        this->line_07_slope = 0;
        this->line_07_startValue = v;
        this->line_07_startPendingRamp();
    } else {
        this->line_07_scheduleNext();
    }
}

void message_11_listin_list_set(const list& v) {
    this->message_11_set_set(v);
}

void message_11_listin_number_set(number v) {
    this->message_11_set_set(v);
}

void message_11_listin_bang_bang() {
    this->message_11_trigger_bang();
}

void line_08_tick_set(number v) {
    this->line_08_output_set(v);

    if ((bool)(this->line_08_isFinished(v))) {
        this->line_08_slope = 0;
        this->line_08_startValue = v;
        this->line_08_startPendingRamp();
    } else {
        this->line_08_scheduleNext();
    }
}

void button_09_bangin_bang() {
    this->button_09_bangval_bang();
}

void numberobj_07_valin_set(number v) {
    this->numberobj_07_value_set(v);
}

void numberobj_07_format_set(number v) {
    this->numberobj_07_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_10_bangin_bang() {
    this->button_10_bangval_bang();
}

void line_09_tick_set(number v) {
    this->line_09_output_set(v);

    if ((bool)(this->line_09_isFinished(v))) {
        this->line_09_slope = 0;
        this->line_09_startValue = v;
        this->line_09_startPendingRamp();
    } else {
        this->line_09_scheduleNext();
    }
}

void message_12_listin_list_set(const list& v) {
    this->message_12_set_set(v);
}

void message_12_listin_number_set(number v) {
    this->message_12_set_set(v);
}

void message_12_listin_bang_bang() {
    this->message_12_trigger_bang();
}

void line_10_tick_set(number v) {
    this->line_10_output_set(v);

    if ((bool)(this->line_10_isFinished(v))) {
        this->line_10_slope = 0;
        this->line_10_startValue = v;
        this->line_10_startPendingRamp();
    } else {
        this->line_10_scheduleNext();
    }
}

void numberobj_08_valin_set(number v) {
    this->numberobj_08_value_set(v);
}

void numberobj_08_format_set(number v) {
    this->numberobj_08_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_09_valin_set(number v) {
    this->numberobj_09_value_set(v);
}

void numberobj_09_format_set(number v) {
    this->numberobj_09_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void slider_07_valin_set(number v) {
    this->slider_07_value_set(v);
}

void button_11_bangin_bang() {
    this->button_11_bangval_bang();
}

void line_11_tick_set(number v) {
    this->line_11_output_set(v);

    if ((bool)(this->line_11_isFinished(v))) {
        this->line_11_slope = 0;
        this->line_11_startValue = v;
        this->line_11_startPendingRamp();
    } else {
        this->line_11_scheduleNext();
    }
}

void message_13_listin_list_set(const list& v) {
    this->message_13_set_set(v);
}

void message_13_listin_number_set(number v) {
    this->message_13_set_set(v);
}

void message_13_listin_bang_bang() {
    this->message_13_trigger_bang();
}

void message_14_listin_list_set(const list& v) {
    this->message_14_set_set(v);
}

void message_14_listin_number_set(number v) {
    this->message_14_set_set(v);
}

void message_14_listin_bang_bang() {
    this->message_14_trigger_bang();
}

void slider_08_valin_set(number v) {
    this->slider_08_value_set(v);
}

void toggle_05_valin_set(number v) {
    this->toggle_05_value_number_set(v);
}

void gaintilde_05_valin_set(number v) {
    this->gaintilde_05_value_set(v);
}

void message_15_listin_list_set(const list& v) {
    this->message_15_set_set(v);
}

void message_15_listin_number_set(number v) {
    this->message_15_set_set(v);
}

void message_15_listin_bang_bang() {
    this->message_15_trigger_bang();
}

void line_12_tick_set(number v) {
    this->line_12_output_set(v);

    if ((bool)(this->line_12_isFinished(v))) {
        this->line_12_slope = 0;
        this->line_12_startValue = v;
        this->line_12_startPendingRamp();
    } else {
        this->line_12_scheduleNext();
    }
}

void numberobj_10_valin_set(number v) {
    this->numberobj_10_value_set(v);
}

void numberobj_10_format_set(number v) {
    this->numberobj_10_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void message_16_listin_list_set(const list& v) {
    this->message_16_set_set(v);
}

void message_16_listin_number_set(number v) {
    this->message_16_set_set(v);
}

void message_16_listin_bang_bang() {
    this->message_16_trigger_bang();
}

void gaintilde_06_valin_set(number v) {
    this->gaintilde_06_value_set(v);
}

void message_17_listin_list_set(const list& v) {
    this->message_17_set_set(v);
}

void message_17_listin_number_set(number v) {
    this->message_17_set_set(v);
}

void message_17_listin_bang_bang() {
    this->message_17_trigger_bang();
}

void line_13_tick_set(number v) {
    this->line_13_output_set(v);

    if ((bool)(this->line_13_isFinished(v))) {
        this->line_13_slope = 0;
        this->line_13_startValue = v;
        this->line_13_startPendingRamp();
    } else {
        this->line_13_scheduleNext();
    }
}

void gaintilde_07_valin_set(number v) {
    this->gaintilde_07_value_set(v);
}

void button_12_bangin_bang() {
    this->button_12_bangval_bang();
}

void numberobj_11_valin_set(number v) {
    this->numberobj_11_value_set(v);
}

void numberobj_11_format_set(number v) {
    this->numberobj_11_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void toggle_06_valin_set(number v) {
    this->toggle_06_value_number_set(v);
}

void metro_02_tick_bang() {
    this->metro_02_tickout_bang();
    this->getEngine()->flushClockEvents(this, -871550838, false);;

    if ((bool)(this->metro_02_on)) {
        this->metro_02_last = this->currenttime();

        {
            this->metro_02_next = this->metro_02_last + this->metro_02_interval;
            this->getEngine()->scheduleClockEvent(this, -871550838, this->metro_02_interval + this->_currentTime);;
        }
    }
}

void button_13_bangin_bang() {
    this->button_13_bangval_bang();
}

void line_14_tick_set(number v) {
    this->line_14_output_set(v);

    if ((bool)(this->line_14_isFinished(v))) {
        this->line_14_slope = 0;
        this->line_14_startValue = v;
        this->line_14_startPendingRamp();
    } else {
        this->line_14_scheduleNext();
    }
}

void gaintilde_08_valin_set(number v) {
    this->gaintilde_08_value_set(v);
}

void button_14_bangin_bang() {
    this->button_14_bangval_bang();
}

void message_18_listin_list_set(const list& v) {
    this->message_18_set_set(v);
}

void message_18_listin_number_set(number v) {
    this->message_18_set_set(v);
}

void message_18_listin_bang_bang() {
    this->message_18_trigger_bang();
}

void numberobj_12_valin_set(number v) {
    this->numberobj_12_value_set(v);
}

void numberobj_12_format_set(number v) {
    this->numberobj_12_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_13_valin_set(number v) {
    this->numberobj_13_value_set(v);
}

void numberobj_13_format_set(number v) {
    this->numberobj_13_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_14_valin_set(number v) {
    this->numberobj_14_value_set(v);
}

void numberobj_14_format_set(number v) {
    this->numberobj_14_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_15_bangin_bang() {
    this->button_15_bangval_bang();
}

void message_19_listin_list_set(const list& v) {
    this->message_19_set_set(v);
}

void message_19_listin_number_set(number v) {
    this->message_19_set_set(v);
}

void message_19_listin_bang_bang() {
    this->message_19_trigger_bang();
}

void numberobj_15_valin_set(number v) {
    this->numberobj_15_value_set(v);
}

void numberobj_15_format_set(number v) {
    this->numberobj_15_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void line_15_tick_set(number v) {
    this->line_15_output_set(v);

    if ((bool)(this->line_15_isFinished(v))) {
        this->line_15_slope = 0;
        this->line_15_startValue = v;
        this->line_15_startPendingRamp();
    } else {
        this->line_15_scheduleNext();
    }
}

void button_16_bangin_bang() {
    this->button_16_bangval_bang();
}

void button_17_bangin_bang() {
    this->button_17_bangval_bang();
}

void numberobj_16_valin_set(number v) {
    this->numberobj_16_value_set(v);
}

void numberobj_16_format_set(number v) {
    this->numberobj_16_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void message_20_listin_list_set(const list& v) {
    this->message_20_set_set(v);
}

void message_20_listin_number_set(number v) {
    this->message_20_set_set(v);
}

void message_20_listin_bang_bang() {
    this->message_20_trigger_bang();
}

void numberobj_17_valin_set(number v) {
    this->numberobj_17_value_set(v);
}

void numberobj_17_format_set(number v) {
    this->numberobj_17_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void line_16_tick_set(number v) {
    this->line_16_output_set(v);

    if ((bool)(this->line_16_isFinished(v))) {
        this->line_16_slope = 0;
        this->line_16_startValue = v;
        this->line_16_startPendingRamp();
    } else {
        this->line_16_scheduleNext();
    }
}

void message_21_listin_list_set(const list& v) {
    this->message_21_set_set(v);
}

void message_21_listin_number_set(number v) {
    this->message_21_set_set(v);
}

void message_21_listin_bang_bang() {
    this->message_21_trigger_bang();
}

void loadbang_08_startupbang_bang() {
    this->loadbang_08_output_bang();
}

void numberobj_18_valin_set(number v) {
    this->numberobj_18_value_set(v);
}

void numberobj_18_format_set(number v) {
    this->numberobj_18_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_18_bangin_bang() {
    this->button_18_bangval_bang();
}

void delay_01_out_bang() {
    this->toggle_09_value_bang_bang();
    this->toggle_08_value_bang_bang();
    this->toggle_07_value_bang_bang();
    this->toggle_06_value_bang_bang();
}

void numberobj_19_valin_set(number v) {
    this->numberobj_19_value_set(v);
}

void numberobj_19_format_set(number v) {
    this->numberobj_19_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void toggle_07_valin_set(number v) {
    this->toggle_07_value_number_set(v);
}

void metro_03_tick_bang() {
    this->metro_03_tickout_bang();
    this->getEngine()->flushClockEvents(this, -1243558069, false);;

    if ((bool)(this->metro_03_on)) {
        this->metro_03_last = this->currenttime();

        {
            this->metro_03_next = this->metro_03_last + this->metro_03_interval;
            this->getEngine()->scheduleClockEvent(this, -1243558069, this->metro_03_interval + this->_currentTime);;
        }
    }
}

void button_19_bangin_bang() {
    this->button_19_bangval_bang();
}

void numberobj_20_valin_set(number v) {
    this->numberobj_20_value_set(v);
}

void numberobj_20_format_set(number v) {
    this->numberobj_20_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_21_valin_set(number v) {
    this->numberobj_21_value_set(v);
}

void numberobj_21_format_set(number v) {
    this->numberobj_21_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_22_valin_set(number v) {
    this->numberobj_22_value_set(v);
}

void numberobj_22_format_set(number v) {
    this->numberobj_22_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void line_17_tick_set(number v) {
    this->line_17_output_set(v);

    if ((bool)(this->line_17_isFinished(v))) {
        this->line_17_slope = 0;
        this->line_17_startValue = v;
        this->line_17_startPendingRamp();
    } else {
        this->line_17_scheduleNext();
    }
}

void button_20_bangin_bang() {
    this->button_20_bangval_bang();
}

void button_21_bangin_bang() {
    this->button_21_bangval_bang();
}

void numberobj_23_valin_set(number v) {
    this->numberobj_23_value_set(v);
}

void numberobj_23_format_set(number v) {
    this->numberobj_23_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void message_22_listin_list_set(const list& v) {
    this->message_22_set_set(v);
}

void message_22_listin_number_set(number v) {
    this->message_22_set_set(v);
}

void message_22_listin_bang_bang() {
    this->message_22_trigger_bang();
}

void numberobj_24_valin_set(number v) {
    this->numberobj_24_value_set(v);
}

void numberobj_24_format_set(number v) {
    this->numberobj_24_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_25_valin_set(number v) {
    this->numberobj_25_value_set(v);
}

void numberobj_25_format_set(number v) {
    this->numberobj_25_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void toggle_08_valin_set(number v) {
    this->toggle_08_value_number_set(v);
}

void metro_04_tick_bang() {
    this->metro_04_tickout_bang();
    this->getEngine()->flushClockEvents(this, -1615565300, false);;

    if ((bool)(this->metro_04_on)) {
        this->metro_04_last = this->currenttime();

        {
            this->metro_04_next = this->metro_04_last + this->metro_04_interval;
            this->getEngine()->scheduleClockEvent(this, -1615565300, this->metro_04_interval + this->_currentTime);;
        }
    }
}

void button_22_bangin_bang() {
    this->button_22_bangval_bang();
}

void numberobj_26_valin_set(number v) {
    this->numberobj_26_value_set(v);
}

void numberobj_26_format_set(number v) {
    this->numberobj_26_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_27_valin_set(number v) {
    this->numberobj_27_value_set(v);
}

void numberobj_27_format_set(number v) {
    this->numberobj_27_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_28_valin_set(number v) {
    this->numberobj_28_value_set(v);
}

void numberobj_28_format_set(number v) {
    this->numberobj_28_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void line_18_tick_set(number v) {
    this->line_18_output_set(v);

    if ((bool)(this->line_18_isFinished(v))) {
        this->line_18_slope = 0;
        this->line_18_startValue = v;
        this->line_18_startPendingRamp();
    } else {
        this->line_18_scheduleNext();
    }
}

void button_23_bangin_bang() {
    this->button_23_bangval_bang();
}

void button_24_bangin_bang() {
    this->button_24_bangval_bang();
}

void numberobj_29_valin_set(number v) {
    this->numberobj_29_value_set(v);
}

void numberobj_29_format_set(number v) {
    this->numberobj_29_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void message_23_listin_list_set(const list& v) {
    this->message_23_set_set(v);
}

void message_23_listin_number_set(number v) {
    this->message_23_set_set(v);
}

void message_23_listin_bang_bang() {
    this->message_23_trigger_bang();
}

void numberobj_30_valin_set(number v) {
    this->numberobj_30_value_set(v);
}

void numberobj_30_format_set(number v) {
    this->numberobj_30_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_31_valin_set(number v) {
    this->numberobj_31_value_set(v);
}

void numberobj_31_format_set(number v) {
    this->numberobj_31_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void toggle_09_valin_set(number v) {
    this->toggle_09_value_number_set(v);
}

void metro_05_tick_bang() {
    this->metro_05_tickout_bang();
    this->getEngine()->flushClockEvents(this, -1987572531, false);;

    if ((bool)(this->metro_05_on)) {
        this->metro_05_last = this->currenttime();

        {
            this->metro_05_next = this->metro_05_last + this->metro_05_interval;
            this->getEngine()->scheduleClockEvent(this, -1987572531, this->metro_05_interval + this->_currentTime);;
        }
    }
}

void button_25_bangin_bang() {
    this->button_25_bangval_bang();
}

void numberobj_32_valin_set(number v) {
    this->numberobj_32_value_set(v);
}

void numberobj_32_format_set(number v) {
    this->numberobj_32_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_33_valin_set(number v) {
    this->numberobj_33_value_set(v);
}

void numberobj_33_format_set(number v) {
    this->numberobj_33_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_34_valin_set(number v) {
    this->numberobj_34_value_set(v);
}

void numberobj_34_format_set(number v) {
    this->numberobj_34_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void line_19_tick_set(number v) {
    this->line_19_output_set(v);

    if ((bool)(this->line_19_isFinished(v))) {
        this->line_19_slope = 0;
        this->line_19_startValue = v;
        this->line_19_startPendingRamp();
    } else {
        this->line_19_scheduleNext();
    }
}

void button_26_bangin_bang() {
    this->button_26_bangval_bang();
}

void button_27_bangin_bang() {
    this->button_27_bangval_bang();
}

void numberobj_35_valin_set(number v) {
    this->numberobj_35_value_set(v);
}

void numberobj_35_format_set(number v) {
    this->numberobj_35_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void message_24_listin_list_set(const list& v) {
    this->message_24_set_set(v);
}

void message_24_listin_number_set(number v) {
    this->message_24_set_set(v);
}

void message_24_listin_bang_bang() {
    this->message_24_trigger_bang();
}

void numberobj_36_valin_set(number v) {
    this->numberobj_36_value_set(v);
}

void numberobj_36_format_set(number v) {
    this->numberobj_36_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void adsr_01_mute_bang() {}

number msToSamps(MillisecondTime ms, number sampleRate) {
    return ms * sampleRate * 0.001;
}

MillisecondTime sampsToMs(SampleIndex samps) {
    return samps * (this->invsr * 1000);
}

Index getMaxBlockSize() const {
    return this->maxvs;
}

number getSampleRate() const {
    return this->sr;
}

bool hasFixedVectorSize() const {
    return false;
}

Index getNumInputChannels() const {
    return 0;
}

Index getNumOutputChannels() const {
    return 2;
}

void allocateDataRefs() {
    this->mtof_tilde_01_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_tilde_01_innerMtoF_buffer->setSampleRate(this->sr);
    this->mtof_tilde_02_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_tilde_02_innerMtoF_buffer->setSampleRate(this->sr);
    this->mtof_tilde_03_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_tilde_03_innerMtoF_buffer->setSampleRate(this->sr);
    this->mtof_tilde_04_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_tilde_04_innerMtoF_buffer->setSampleRate(this->sr);
    this->cycle_tilde_01_buffer->requestSize(16384, 1);
    this->cycle_tilde_01_buffer->setSampleRate(this->sr);
    this->data_01_buffer = this->data_01_buffer->allocateIfNeeded();
    this->groove_01_buffer = this->groove_01_buffer->allocateIfNeeded();

    if (this->background->hasRequestedSize()) {
        if (this->background->wantsFill())
            this->zeroDataRef(this->background);

        this->getEngine()->sendDataRefUpdated(0);
    }

    this->data_02_buffer = this->data_02_buffer->allocateIfNeeded();
    this->multibuffertilde_01_buffer0 = this->multibuffertilde_01_buffer0->allocateIfNeeded();

    if (this->b_hit1_wav->hasRequestedSize()) {
        if (this->b_hit1_wav->wantsFill())
            this->zeroDataRef(this->b_hit1_wav);

        this->getEngine()->sendDataRefUpdated(1);
    }

    this->data_03_buffer = this->data_03_buffer->allocateIfNeeded();
    this->groove_04_buffer = this->groove_04_buffer->allocateIfNeeded();

    if (this->bigbot->hasRequestedSize()) {
        if (this->bigbot->wantsFill())
            this->zeroDataRef(this->bigbot);

        this->getEngine()->sendDataRefUpdated(2);
    }

    this->data_04_buffer = this->data_04_buffer->allocateIfNeeded();
    this->groove_02_buffer = this->groove_02_buffer->allocateIfNeeded();

    if (this->littlebot->hasRequestedSize()) {
        if (this->littlebot->wantsFill())
            this->zeroDataRef(this->littlebot);

        this->getEngine()->sendDataRefUpdated(3);
    }

    this->data_05_buffer = this->data_05_buffer->allocateIfNeeded();
    this->multibuffertilde_01_buffer1 = this->multibuffertilde_01_buffer1->allocateIfNeeded();

    if (this->b_hit3_wav->hasRequestedSize()) {
        if (this->b_hit3_wav->wantsFill())
            this->zeroDataRef(this->b_hit3_wav);

        this->getEngine()->sendDataRefUpdated(4);
    }

    this->data_06_buffer = this->data_06_buffer->allocateIfNeeded();
    this->multibuffertilde_01_buffer2 = this->multibuffertilde_01_buffer2->allocateIfNeeded();

    if (this->b_hit2_wav->hasRequestedSize()) {
        if (this->b_hit2_wav->wantsFill())
            this->zeroDataRef(this->b_hit2_wav);

        this->getEngine()->sendDataRefUpdated(5);
    }

    this->data_07_buffer = this->data_07_buffer->allocateIfNeeded();
    this->multibuffertilde_01_buffer3 = this->multibuffertilde_01_buffer3->allocateIfNeeded();

    if (this->b_hit4_wav->hasRequestedSize()) {
        if (this->b_hit4_wav->wantsFill())
            this->zeroDataRef(this->b_hit4_wav);

        this->getEngine()->sendDataRefUpdated(6);
    }

    this->data_08_buffer = this->data_08_buffer->allocateIfNeeded();
    this->multibuffertilde_01_buffer4 = this->multibuffertilde_01_buffer4->allocateIfNeeded();

    if (this->b_hit5_wav->hasRequestedSize()) {
        if (this->b_hit5_wav->wantsFill())
            this->zeroDataRef(this->b_hit5_wav);

        this->getEngine()->sendDataRefUpdated(7);
    }

    this->mtof_tilde_01_innerMtoF_buffer = this->mtof_tilde_01_innerMtoF_buffer->allocateIfNeeded();
    this->mtof_tilde_02_innerMtoF_buffer = this->mtof_tilde_02_innerMtoF_buffer->allocateIfNeeded();
    this->mtof_tilde_03_innerMtoF_buffer = this->mtof_tilde_03_innerMtoF_buffer->allocateIfNeeded();
    this->mtof_tilde_04_innerMtoF_buffer = this->mtof_tilde_04_innerMtoF_buffer->allocateIfNeeded();

    if (this->RNBODefaultMtofLookupTable256->hasRequestedSize()) {
        if (this->RNBODefaultMtofLookupTable256->wantsFill())
            this->fillRNBODefaultMtofLookupTable256(this->RNBODefaultMtofLookupTable256);

        this->getEngine()->sendDataRefUpdated(8);
    }

    this->cycle_tilde_01_buffer = this->cycle_tilde_01_buffer->allocateIfNeeded();

    if (this->RNBODefaultSinus->hasRequestedSize()) {
        if (this->RNBODefaultSinus->wantsFill())
            this->fillRNBODefaultSinus(this->RNBODefaultSinus);

        this->getEngine()->sendDataRefUpdated(9);
    }
}

void initializeObjects() {
    this->data_01_init();
    this->data_02_init();
    this->message_01_init();
    this->data_03_init();
    this->data_04_init();
    this->data_05_init();
    this->data_06_init();
    this->data_07_init();
    this->data_08_init();
    this->gaintilde_01_init();
    this->random_01_nz_init();
    this->gaintilde_02_init();
    this->numberobj_01_init();
    this->message_02_init();
    this->message_03_init();
    this->numberobj_02_init();
    this->mtof_tilde_01_innerScala_init();
    this->mtof_tilde_01_init();
    this->message_04_init();
    this->numberobj_03_init();
    this->gaintilde_03_init();
    this->random_02_nz_init();
    this->message_05_init();
    this->message_06_init();
    this->numberobj_04_init();
    this->gaintilde_04_init();
    this->numberobj_05_init();
    this->random_03_nz_init();
    this->mtof_tilde_02_innerScala_init();
    this->mtof_tilde_02_init();
    this->message_07_init();
    this->message_08_init();
    this->numberobj_06_init();
    this->message_09_init();
    this->mtof_tilde_03_innerScala_init();
    this->mtof_tilde_03_init();
    this->message_10_init();
    this->message_11_init();
    this->random_04_nz_init();
    this->numberobj_07_init();
    this->random_05_nz_init();
    this->message_12_init();
    this->numberobj_08_init();
    this->numberobj_09_init();
    this->mtof_tilde_04_innerScala_init();
    this->mtof_tilde_04_init();
    this->message_13_init();
    this->message_14_init();
    this->gaintilde_05_init();
    this->message_15_init();
    this->numberobj_10_init();
    this->message_16_init();
    this->gaintilde_06_init();
    this->message_17_init();
    this->gaintilde_07_init();
    this->random_06_nz_init();
    this->numberobj_11_init();
    this->random_07_nz_init();
    this->random_08_nz_init();
    this->gaintilde_08_init();
    this->message_18_init();
    this->numberobj_12_init();
    this->numberobj_13_init();
    this->numberobj_14_init();
    this->message_19_init();
    this->numberobj_15_init();
    this->random_09_nz_init();
    this->numberobj_16_init();
    this->message_20_init();
    this->numberobj_17_init();
    this->message_21_init();
    this->numberobj_18_init();
    this->random_10_nz_init();
    this->numberobj_19_init();
    this->random_11_nz_init();
    this->numberobj_20_init();
    this->numberobj_21_init();
    this->numberobj_22_init();
    this->random_12_nz_init();
    this->numberobj_23_init();
    this->message_22_init();
    this->numberobj_24_init();
    this->random_13_nz_init();
    this->numberobj_25_init();
    this->random_14_nz_init();
    this->numberobj_26_init();
    this->numberobj_27_init();
    this->numberobj_28_init();
    this->random_15_nz_init();
    this->numberobj_29_init();
    this->message_23_init();
    this->numberobj_30_init();
    this->random_16_nz_init();
    this->numberobj_31_init();
    this->random_17_nz_init();
    this->numberobj_32_init();
    this->numberobj_33_init();
    this->numberobj_34_init();
    this->random_18_nz_init();
    this->numberobj_35_init();
    this->message_24_init();
    this->numberobj_36_init();
}

void sendOutlet(OutletIndex index, ParameterValue value) {
    this->getEngine()->sendOutlet(this, index, value);
}

void startup() {
    this->updateTime(this->getEngine()->getCurrentTime());
    this->getEngine()->scheduleClockEvent(this, 1696335371, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, -1735137014, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, -871642103, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, 1673828112, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, -30654451, 0 + this->_currentTime);;

    if ((bool)(this->metro_01_on))
        this->metro_01_on_set(1);

    this->getEngine()->scheduleClockEvent(this, -1757644273, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, 832840460, 0 + this->_currentTime);;

    if ((bool)(this->metro_02_on))
        this->metro_02_on_set(1);

    this->getEngine()->scheduleClockEvent(this, -894149362, 0 + this->_currentTime);;

    if ((bool)(this->metro_03_on))
        this->metro_03_on_set(1);

    if ((bool)(this->metro_04_on))
        this->metro_04_on_set(1);

    if ((bool)(this->metro_05_on))
        this->metro_05_on_set(1);

    this->timevalue_01_sendValue();
    this->timevalue_02_sendValue();
    this->timevalue_03_sendValue();
    this->timevalue_04_sendValue();
    this->timevalue_05_sendValue();

    {
        this->scheduleParamInit(0, 0);
    }

    {
        this->scheduleParamInit(1, 0);
    }

    {
        this->scheduleParamInit(2, 0);
    }

    {
        this->scheduleParamInit(3, 0);
    }

    this->processParamInitEvents();
}

number param_01_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void groove_04_rate_bang_bang() {
    this->groove_04_changeIncomingInSamples = this->sampleOffsetIntoNextAudioBuffer + 1;
    this->groove_04_incomingChange = 1;
}

void groove_02_rate_bang_bang() {
    this->groove_02_changeIncomingInSamples = this->sampleOffsetIntoNextAudioBuffer + 1;
    this->groove_02_incomingChange = 1;
}

void button_04_output_bang() {
    this->groove_04_rate_bang_bang();
    this->groove_02_rate_bang_bang();
}

void button_04_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-117"), this->_currentTime);;
    this->button_04_output_bang();
}

number line_13_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_13_time_set(number v) {
    v = this->line_13_time_constrain(v);
    this->line_13_time = v;
}

void dspexpr_07_in2_set(number v) {
    this->dspexpr_07_in2 = v;
}

void slider_08_output_set(number v) {
    this->dspexpr_07_in2_set(v);
}

void slider_08_value_set(number v) {
    this->slider_08_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 2, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-53"), v, this->_currentTime);
    this->slider_08_output_set(value);
}

void slider_08_input_set(number v) {
    this->slider_08_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 2),
        0,
        0 + 2,
        0,
        128,
        1
    ));
}

void line_13_output_set(number v) {
    this->line_13_output = v;
    this->slider_08_input_set(v);
}

void line_13_stop_bang() {
    this->getEngine()->flushClockEvents(this, 2006462159, false);;
    this->line_13_pendingRamps->length = 0;
    this->line_13_startValue = this->line_13_output;
    this->line_13_slope = 0;
    this->line_13__time = 0;
    this->line_13_time_set(0);
}

number line_13_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_13_grain_set(number v) {
    v = this->line_13_grain_constrain(v);
    this->line_13_grain = v;

    if ((bool)(!(bool)(this->line_13_isFinished(this->line_13_startValue)))) {
        this->getEngine()->flushClockEvents(this, 2006462159, false);;
        this->line_13_scheduleNext();
    }
}

void numberobj_33_output_set(number ) {}

void numberobj_33_value_set(number v) {
    this->numberobj_33_value_setter(v);
    v = this->numberobj_33_value;
    number localvalue = v;

    if (this->numberobj_33_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-197"), localvalue, this->_currentTime);
    this->numberobj_33_output_set(localvalue);
}

void counter_08_maximum_set(number v) {
    this->counter_08_maximum = v;

    if (this->counter_08_count > this->counter_08_maximum)
        this->counter_08_count = this->counter_08_minimum;
}

void random_16_result_set(number v) {
    this->numberobj_33_value_set(v);
    this->counter_08_maximum_set(v);
}

void random_16_trigger_bang() {
    this->random_16_result_set(rnbo_abs(this->random_16_nz_next()) * this->random_16_limit);
}

void metro_05_on_set(number v) {
    this->metro_05_on = v;
    this->getEngine()->flushClockEvents(this, -1987572531, false);;

    if ((bool)(v)) {
        {
            this->getEngine()->scheduleClockEvent(this, -1987572531, 0 + this->_currentTime);;
        }
    }
}

void toggle_09_output_set(number v) {
    this->metro_05_on_set(v);
}

void toggle_09_value_number_set(number v) {
    this->toggle_09_value_number_setter(v);
    v = this->toggle_09_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-200"), v, this->_currentTime);
    this->toggle_09_output_set(v);
}

void toggle_09_value_bang_bang() {
    number val = (this->toggle_09_value_number == 1 ? 0 : 1);
    this->toggle_09_value_number_set(val);
}

void numberobj_27_output_set(number ) {}

void numberobj_27_value_set(number v) {
    this->numberobj_27_value_setter(v);
    v = this->numberobj_27_value;
    number localvalue = v;

    if (this->numberobj_27_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-173"), localvalue, this->_currentTime);
    this->numberobj_27_output_set(localvalue);
}

void counter_06_maximum_set(number v) {
    this->counter_06_maximum = v;

    if (this->counter_06_count > this->counter_06_maximum)
        this->counter_06_count = this->counter_06_minimum;
}

void random_13_result_set(number v) {
    this->numberobj_27_value_set(v);
    this->counter_06_maximum_set(v);
}

void random_13_trigger_bang() {
    this->random_13_result_set(rnbo_abs(this->random_13_nz_next()) * this->random_13_limit);
}

void metro_04_on_set(number v) {
    this->metro_04_on = v;
    this->getEngine()->flushClockEvents(this, -1615565300, false);;

    if ((bool)(v)) {
        {
            this->getEngine()->scheduleClockEvent(this, -1615565300, 0 + this->_currentTime);;
        }
    }
}

void toggle_08_output_set(number v) {
    this->metro_04_on_set(v);
}

void toggle_08_value_number_set(number v) {
    this->toggle_08_value_number_setter(v);
    v = this->toggle_08_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-176"), v, this->_currentTime);
    this->toggle_08_output_set(v);
}

void toggle_08_value_bang_bang() {
    number val = (this->toggle_08_value_number == 1 ? 0 : 1);
    this->toggle_08_value_number_set(val);
}

void numberobj_21_output_set(number ) {}

void numberobj_21_value_set(number v) {
    this->numberobj_21_value_setter(v);
    v = this->numberobj_21_value;
    number localvalue = v;

    if (this->numberobj_21_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-149"), localvalue, this->_currentTime);
    this->numberobj_21_output_set(localvalue);
}

void counter_04_maximum_set(number v) {
    this->counter_04_maximum = v;

    if (this->counter_04_count > this->counter_04_maximum)
        this->counter_04_count = this->counter_04_minimum;
}

void random_10_result_set(number v) {
    this->numberobj_21_value_set(v);
    this->counter_04_maximum_set(v);
}

void random_10_trigger_bang() {
    this->random_10_result_set(rnbo_abs(this->random_10_nz_next()) * this->random_10_limit);
}

void metro_03_on_set(number v) {
    this->metro_03_on = v;
    this->getEngine()->flushClockEvents(this, -1243558069, false);;

    if ((bool)(v)) {
        {
            this->getEngine()->scheduleClockEvent(this, -1243558069, 0 + this->_currentTime);;
        }
    }
}

void toggle_07_output_set(number v) {
    this->metro_03_on_set(v);
}

void toggle_07_value_number_set(number v) {
    this->toggle_07_value_number_setter(v);
    v = this->toggle_07_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-152"), v, this->_currentTime);
    this->toggle_07_output_set(v);
}

void toggle_07_value_bang_bang() {
    number val = (this->toggle_07_value_number == 1 ? 0 : 1);
    this->toggle_07_value_number_set(val);
}

void numberobj_14_output_set(number ) {}

void numberobj_14_value_set(number v) {
    this->numberobj_14_value_setter(v);
    v = this->numberobj_14_value;
    number localvalue = v;

    if (this->numberobj_14_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-85"), localvalue, this->_currentTime);
    this->numberobj_14_output_set(localvalue);
}

void counter_02_maximum_set(number v) {
    this->counter_02_maximum = v;

    if (this->counter_02_count > this->counter_02_maximum)
        this->counter_02_count = this->counter_02_minimum;
}

void random_06_result_set(number v) {
    this->numberobj_14_value_set(v);
    this->counter_02_maximum_set(v);
}

void random_06_trigger_bang() {
    this->random_06_result_set(rnbo_abs(this->random_06_nz_next()) * this->random_06_limit);
}

void metro_02_on_set(number v) {
    this->metro_02_on = v;
    this->getEngine()->flushClockEvents(this, -871550838, false);;

    if ((bool)(v)) {
        {
            this->getEngine()->scheduleClockEvent(this, -871550838, 0 + this->_currentTime);;
        }
    }
}

void toggle_06_output_set(number v) {
    this->metro_02_on_set(v);
}

void toggle_06_value_number_set(number v) {
    this->toggle_06_value_number_setter(v);
    v = this->toggle_06_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-80"), v, this->_currentTime);
    this->toggle_06_output_set(v);
}

void toggle_06_value_bang_bang() {
    number val = (this->toggle_06_value_number == 1 ? 0 : 1);
    this->toggle_06_value_number_set(val);
}

void metro_01_on_set(number v) {
    this->metro_01_on = v;
    this->getEngine()->flushClockEvents(this, 1935387534, false);;

    if ((bool)(v)) {
        {
            this->getEngine()->scheduleClockEvent(this, 1935387534, 0 + this->_currentTime);;
        }
    }
}

void toggle_04_output_set(number v) {
    this->metro_01_on_set(v);
}

void toggle_04_value_number_set(number v) {
    this->toggle_04_value_number_setter(v);
    v = this->toggle_04_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-73"), v, this->_currentTime);
    this->toggle_04_output_set(v);
}

void toggle_04_value_bang_bang() {
    number val = (this->toggle_04_value_number == 1 ? 0 : 1);
    this->toggle_04_value_number_set(val);
}

void button_01_output_bang() {
    this->toggle_04_value_bang_bang();
}

void button_01_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-8"), this->_currentTime);;
    this->button_01_output_bang();
}

void button_12_output_bang() {
    this->random_16_trigger_bang();
    this->toggle_09_value_bang_bang();
    this->random_13_trigger_bang();
    this->toggle_08_value_bang_bang();
    this->random_10_trigger_bang();
    this->toggle_07_value_bang_bang();
    this->random_06_trigger_bang();
    this->toggle_06_value_bang_bang();
    this->button_01_bangval_bang();
}

void button_12_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-7"), this->_currentTime);;
    this->button_12_output_bang();
}

void line_13_end_bang() {
    this->button_12_bangval_bang();
}

void line_13_target_set(const list& v) {
    this->line_13_target = jsCreateListCopy(v);
    this->line_13_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_13__time = this->line_13_time;
        this->line_13_time_set(0);

        if ((bool)(this->line_13__time)) {
            this->line_13_startRamp(v[0], this->line_13__time);
        } else {
            this->line_13_output_set(v[0]);
            this->line_13_startValue = v[0];
            this->line_13_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_13_time_set(0);
        this->line_13__time = (v[1] < 0 ? 0 : v[1]);
        this->line_13_startRamp(v[0], this->line_13__time);
    } else if (v->length == 3) {
        this->line_13_time_set(0);
        this->line_13_grain_set(v[2]);
        this->line_13__time = (v[1] < 0 ? 0 : v[1]);
        this->line_13_startRamp(v[0], this->line_13__time);
    } else {
        this->line_13_time_set(0);
        this->line_13_pendingRamps = jsCreateListCopy(v);
        this->line_13_startPendingRamp();
    }
}

void message_17_out_set(const list& v) {
    this->line_13_target_set(v);
}

void message_17_trigger_bang() {
    this->message_17_out_set(this->message_17_set);
}

number line_12_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_12_time_set(number v) {
    v = this->line_12_time_constrain(v);
    this->line_12_time = v;
}

void dspexpr_06_in2_set(number v) {
    this->dspexpr_06_in2 = v;
}

void slider_06_output_set(number v) {
    this->dspexpr_06_in2_set(v);
}

void slider_06_value_set(number v) {
    this->slider_06_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 2, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-51"), v, this->_currentTime);
    this->slider_06_output_set(value);
}

void slider_06_input_set(number v) {
    this->slider_06_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 2),
        0,
        0 + 2,
        0,
        128,
        1
    ));
}

void line_12_output_set(number v) {
    this->line_12_output = v;
    this->slider_06_input_set(v);
}

void line_12_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1916497906, false);;
    this->line_12_pendingRamps->length = 0;
    this->line_12_startValue = this->line_12_output;
    this->line_12_slope = 0;
    this->line_12__time = 0;
    this->line_12_time_set(0);
}

number line_12_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_12_grain_set(number v) {
    v = this->line_12_grain_constrain(v);
    this->line_12_grain = v;

    if ((bool)(!(bool)(this->line_12_isFinished(this->line_12_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1916497906, false);;
        this->line_12_scheduleNext();
    }
}

void line_12_end_bang() {}

void line_12_target_set(const list& v) {
    this->line_12_target = jsCreateListCopy(v);
    this->line_12_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_12__time = this->line_12_time;
        this->line_12_time_set(0);

        if ((bool)(this->line_12__time)) {
            this->line_12_startRamp(v[0], this->line_12__time);
        } else {
            this->line_12_output_set(v[0]);
            this->line_12_startValue = v[0];
            this->line_12_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_12_time_set(0);
        this->line_12__time = (v[1] < 0 ? 0 : v[1]);
        this->line_12_startRamp(v[0], this->line_12__time);
    } else if (v->length == 3) {
        this->line_12_time_set(0);
        this->line_12_grain_set(v[2]);
        this->line_12__time = (v[1] < 0 ? 0 : v[1]);
        this->line_12_startRamp(v[0], this->line_12__time);
    } else {
        this->line_12_time_set(0);
        this->line_12_pendingRamps = jsCreateListCopy(v);
        this->line_12_startPendingRamp();
    }
}

void message_15_out_set(const list& v) {
    this->line_12_target_set(v);
}

void message_15_trigger_bang() {
    this->message_15_out_set(this->message_15_set);
}

number line_10_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_10_time_set(number v) {
    v = this->line_10_time_constrain(v);
    this->line_10_time = v;
}

void dspexpr_04_in2_set(number v) {
    this->dspexpr_04_in2 = v;
}

void slider_04_output_set(number v) {
    this->dspexpr_04_in2_set(v);
}

void slider_04_value_set(number v) {
    this->slider_04_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 2, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-49"), v, this->_currentTime);
    this->slider_04_output_set(value);
}

void slider_04_input_set(number v) {
    this->slider_04_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 2),
        0,
        0 + 2,
        0,
        128,
        1
    ));
}

void line_10_output_set(number v) {
    this->line_10_output = v;
    this->slider_04_input_set(v);
}

void line_10_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1544490675, false);;
    this->line_10_pendingRamps->length = 0;
    this->line_10_startValue = this->line_10_output;
    this->line_10_slope = 0;
    this->line_10__time = 0;
    this->line_10_time_set(0);
}

number line_10_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_10_grain_set(number v) {
    v = this->line_10_grain_constrain(v);
    this->line_10_grain = v;

    if ((bool)(!(bool)(this->line_10_isFinished(this->line_10_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1544490675, false);;
        this->line_10_scheduleNext();
    }
}

void line_10_end_bang() {}

void line_10_target_set(const list& v) {
    this->line_10_target = jsCreateListCopy(v);
    this->line_10_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_10__time = this->line_10_time;
        this->line_10_time_set(0);

        if ((bool)(this->line_10__time)) {
            this->line_10_startRamp(v[0], this->line_10__time);
        } else {
            this->line_10_output_set(v[0]);
            this->line_10_startValue = v[0];
            this->line_10_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_10_time_set(0);
        this->line_10__time = (v[1] < 0 ? 0 : v[1]);
        this->line_10_startRamp(v[0], this->line_10__time);
    } else if (v->length == 3) {
        this->line_10_time_set(0);
        this->line_10_grain_set(v[2]);
        this->line_10__time = (v[1] < 0 ? 0 : v[1]);
        this->line_10_startRamp(v[0], this->line_10__time);
    } else {
        this->line_10_time_set(0);
        this->line_10_pendingRamps = jsCreateListCopy(v);
        this->line_10_startPendingRamp();
    }
}

void message_12_out_set(const list& v) {
    this->line_10_target_set(v);
}

void message_12_trigger_bang() {
    this->message_12_out_set(this->message_12_set);
}

number line_07_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_07_time_set(number v) {
    v = this->line_07_time_constrain(v);
    this->line_07_time = v;
}

void dspexpr_02_in2_set(number v) {
    this->dspexpr_02_in2 = v;
}

void slider_02_output_set(number v) {
    this->dspexpr_02_in2_set(v);
}

void slider_02_value_set(number v) {
    this->slider_02_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 2, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-48"), v, this->_currentTime);
    this->slider_02_output_set(value);
}

void slider_02_input_set(number v) {
    this->slider_02_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 2),
        0,
        0 + 2,
        0,
        128,
        1
    ));
}

void line_07_output_set(number v) {
    this->line_07_output = v;
    this->slider_02_input_set(v);
}

void line_07_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1172483444, false);;
    this->line_07_pendingRamps->length = 0;
    this->line_07_startValue = this->line_07_output;
    this->line_07_slope = 0;
    this->line_07__time = 0;
    this->line_07_time_set(0);
}

number line_07_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_07_grain_set(number v) {
    v = this->line_07_grain_constrain(v);
    this->line_07_grain = v;

    if ((bool)(!(bool)(this->line_07_isFinished(this->line_07_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1172483444, false);;
        this->line_07_scheduleNext();
    }
}

void line_07_end_bang() {}

void line_07_target_set(const list& v) {
    this->line_07_target = jsCreateListCopy(v);
    this->line_07_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_07__time = this->line_07_time;
        this->line_07_time_set(0);

        if ((bool)(this->line_07__time)) {
            this->line_07_startRamp(v[0], this->line_07__time);
        } else {
            this->line_07_output_set(v[0]);
            this->line_07_startValue = v[0];
            this->line_07_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_07_time_set(0);
        this->line_07__time = (v[1] < 0 ? 0 : v[1]);
        this->line_07_startRamp(v[0], this->line_07__time);
    } else if (v->length == 3) {
        this->line_07_time_set(0);
        this->line_07_grain_set(v[2]);
        this->line_07__time = (v[1] < 0 ? 0 : v[1]);
        this->line_07_startRamp(v[0], this->line_07__time);
    } else {
        this->line_07_time_set(0);
        this->line_07_pendingRamps = jsCreateListCopy(v);
        this->line_07_startPendingRamp();
    }
}

void message_10_out_set(const list& v) {
    this->line_07_target_set(v);
}

void message_10_trigger_bang() {
    this->message_10_out_set(this->message_10_set);
}

number line_06_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_06_time_set(number v) {
    v = this->line_06_time_constrain(v);
    this->line_06_time = v;
}

void numberobj_10_output_set(number ) {}

void numberobj_10_value_set(number v) {
    this->numberobj_10_value_setter(v);
    v = this->numberobj_10_value;
    number localvalue = v;

    if (this->numberobj_10_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-37"), localvalue, this->_currentTime);
    this->numberobj_10_output_set(localvalue);
}

void mtof_tilde_04_midivalue_set(number v) {
    this->mtof_tilde_04_midivalue = v;
}

void slider_07_output_set(number v) {
    this->numberobj_10_value_set(v);
    this->mtof_tilde_04_midivalue_set(v);
}

void slider_07_value_set(number v) {
    this->slider_07_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 128, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-38"), v, this->_currentTime);
    this->slider_07_output_set(value);
}

void slider_07_input_set(number v) {
    this->slider_07_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 128),
        0,
        0 + 128,
        0,
        128,
        1
    ));
}

void line_06_output_set(number v) {
    this->line_06_output = v;
    this->slider_07_input_set(v);
}

void line_06_stop_bang() {
    this->getEngine()->flushClockEvents(this, 518433235, false);;
    this->line_06_pendingRamps->length = 0;
    this->line_06_startValue = this->line_06_output;
    this->line_06_slope = 0;
    this->line_06__time = 0;
    this->line_06_time_set(0);
}

number line_06_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_06_grain_set(number v) {
    v = this->line_06_grain_constrain(v);
    this->line_06_grain = v;

    if ((bool)(!(bool)(this->line_06_isFinished(this->line_06_startValue)))) {
        this->getEngine()->flushClockEvents(this, 518433235, false);;
        this->line_06_scheduleNext();
    }
}

void line_06_end_bang() {}

void line_06_target_set(const list& v) {
    this->line_06_target = jsCreateListCopy(v);
    this->line_06_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_06__time = this->line_06_time;
        this->line_06_time_set(0);

        if ((bool)(this->line_06__time)) {
            this->line_06_startRamp(v[0], this->line_06__time);
        } else {
            this->line_06_output_set(v[0]);
            this->line_06_startValue = v[0];
            this->line_06_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_06_time_set(0);
        this->line_06__time = (v[1] < 0 ? 0 : v[1]);
        this->line_06_startRamp(v[0], this->line_06__time);
    } else if (v->length == 3) {
        this->line_06_time_set(0);
        this->line_06_grain_set(v[2]);
        this->line_06__time = (v[1] < 0 ? 0 : v[1]);
        this->line_06_startRamp(v[0], this->line_06__time);
    } else {
        this->line_06_time_set(0);
        this->line_06_pendingRamps = jsCreateListCopy(v);
        this->line_06_startPendingRamp();
    }
}

void message_09_out_set(const list& v) {
    this->line_06_target_set(v);
}

void message_09_trigger_bang() {
    this->message_09_out_set(this->message_09_set);
}

number line_05_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_05_time_set(number v) {
    v = this->line_05_time_constrain(v);
    this->line_05_time = v;
}

void numberobj_07_output_set(number ) {}

void numberobj_07_value_set(number v) {
    this->numberobj_07_value_setter(v);
    v = this->numberobj_07_value;
    number localvalue = v;

    if (this->numberobj_07_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-34"), localvalue, this->_currentTime);
    this->numberobj_07_output_set(localvalue);
}

void mtof_tilde_03_midivalue_set(number v) {
    this->mtof_tilde_03_midivalue = v;
}

void slider_05_output_set(number v) {
    this->numberobj_07_value_set(v);
    this->mtof_tilde_03_midivalue_set(v);
}

void slider_05_value_set(number v) {
    this->slider_05_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 128, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-35"), v, this->_currentTime);
    this->slider_05_output_set(value);
}

void slider_05_input_set(number v) {
    this->slider_05_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 128),
        0,
        0 + 128,
        0,
        128,
        1
    ));
}

void line_05_output_set(number v) {
    this->line_05_output = v;
    this->slider_05_input_set(v);
}

void line_05_stop_bang() {
    this->getEngine()->flushClockEvents(this, 890440466, false);;
    this->line_05_pendingRamps->length = 0;
    this->line_05_startValue = this->line_05_output;
    this->line_05_slope = 0;
    this->line_05__time = 0;
    this->line_05_time_set(0);
}

number line_05_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_05_grain_set(number v) {
    v = this->line_05_grain_constrain(v);
    this->line_05_grain = v;

    if ((bool)(!(bool)(this->line_05_isFinished(this->line_05_startValue)))) {
        this->getEngine()->flushClockEvents(this, 890440466, false);;
        this->line_05_scheduleNext();
    }
}

void line_05_end_bang() {}

void line_05_target_set(const list& v) {
    this->line_05_target = jsCreateListCopy(v);
    this->line_05_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_05__time = this->line_05_time;
        this->line_05_time_set(0);

        if ((bool)(this->line_05__time)) {
            this->line_05_startRamp(v[0], this->line_05__time);
        } else {
            this->line_05_output_set(v[0]);
            this->line_05_startValue = v[0];
            this->line_05_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_05_time_set(0);
        this->line_05__time = (v[1] < 0 ? 0 : v[1]);
        this->line_05_startRamp(v[0], this->line_05__time);
    } else if (v->length == 3) {
        this->line_05_time_set(0);
        this->line_05_grain_set(v[2]);
        this->line_05__time = (v[1] < 0 ? 0 : v[1]);
        this->line_05_startRamp(v[0], this->line_05__time);
    } else {
        this->line_05_time_set(0);
        this->line_05_pendingRamps = jsCreateListCopy(v);
        this->line_05_startPendingRamp();
    }
}

void message_08_out_set(const list& v) {
    this->line_05_target_set(v);
}

void message_08_trigger_bang() {
    this->message_08_out_set(this->message_08_set);
}

number line_04_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_04_time_set(number v) {
    v = this->line_04_time_constrain(v);
    this->line_04_time = v;
}

void numberobj_06_output_set(number ) {}

void numberobj_06_value_set(number v) {
    this->numberobj_06_value_setter(v);
    v = this->numberobj_06_value;
    number localvalue = v;

    if (this->numberobj_06_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-31"), localvalue, this->_currentTime);
    this->numberobj_06_output_set(localvalue);
}

void mtof_tilde_02_midivalue_set(number v) {
    this->mtof_tilde_02_midivalue = v;
}

void slider_03_output_set(number v) {
    this->numberobj_06_value_set(v);
    this->mtof_tilde_02_midivalue_set(v);
}

void slider_03_value_set(number v) {
    this->slider_03_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 128, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-32"), v, this->_currentTime);
    this->slider_03_output_set(value);
}

void slider_03_input_set(number v) {
    this->slider_03_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 128),
        0,
        0 + 128,
        0,
        128,
        1
    ));
}

void line_04_output_set(number v) {
    this->line_04_output = v;
    this->slider_03_input_set(v);
}

void line_04_stop_bang() {
    this->getEngine()->flushClockEvents(this, 1262447697, false);;
    this->line_04_pendingRamps->length = 0;
    this->line_04_startValue = this->line_04_output;
    this->line_04_slope = 0;
    this->line_04__time = 0;
    this->line_04_time_set(0);
}

number line_04_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_04_grain_set(number v) {
    v = this->line_04_grain_constrain(v);
    this->line_04_grain = v;

    if ((bool)(!(bool)(this->line_04_isFinished(this->line_04_startValue)))) {
        this->getEngine()->flushClockEvents(this, 1262447697, false);;
        this->line_04_scheduleNext();
    }
}

void line_04_end_bang() {}

void line_04_target_set(const list& v) {
    this->line_04_target = jsCreateListCopy(v);
    this->line_04_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_04__time = this->line_04_time;
        this->line_04_time_set(0);

        if ((bool)(this->line_04__time)) {
            this->line_04_startRamp(v[0], this->line_04__time);
        } else {
            this->line_04_output_set(v[0]);
            this->line_04_startValue = v[0];
            this->line_04_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_04_time_set(0);
        this->line_04__time = (v[1] < 0 ? 0 : v[1]);
        this->line_04_startRamp(v[0], this->line_04__time);
    } else if (v->length == 3) {
        this->line_04_time_set(0);
        this->line_04_grain_set(v[2]);
        this->line_04__time = (v[1] < 0 ? 0 : v[1]);
        this->line_04_startRamp(v[0], this->line_04__time);
    } else {
        this->line_04_time_set(0);
        this->line_04_pendingRamps = jsCreateListCopy(v);
        this->line_04_startPendingRamp();
    }
}

void message_06_out_set(const list& v) {
    this->line_04_target_set(v);
}

void message_06_trigger_bang() {
    this->message_06_out_set(this->message_06_set);
}

number line_02_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_02_time_set(number v) {
    v = this->line_02_time_constrain(v);
    this->line_02_time = v;
}

void numberobj_03_output_set(number ) {}

void numberobj_03_value_set(number v) {
    this->numberobj_03_value_setter(v);
    v = this->numberobj_03_value;
    number localvalue = v;

    if (this->numberobj_03_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-30"), localvalue, this->_currentTime);
    this->numberobj_03_output_set(localvalue);
}

void mtof_tilde_01_midivalue_set(number v) {
    this->mtof_tilde_01_midivalue = v;
}

void slider_01_output_set(number v) {
    this->numberobj_03_value_set(v);
    this->mtof_tilde_01_midivalue_set(v);
}

void slider_01_value_set(number v) {
    this->slider_01_value = v;
    number value;

    {
        value = this->scale(v, 0, 128, 0, 0 + 128, 1) * 1;
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("slider_obj-25"), v, this->_currentTime);
    this->slider_01_output_set(value);
}

void slider_01_input_set(number v) {
    this->slider_01_value_set(this->scale(
        this->__wrapped_op_clamp(this->safediv(v, 1), 0, 0 + 128),
        0,
        0 + 128,
        0,
        128,
        1
    ));
}

void line_02_output_set(number v) {
    this->line_02_output = v;
    this->slider_01_input_set(v);
}

void line_02_stop_bang() {
    this->getEngine()->flushClockEvents(this, 1634454928, false);;
    this->line_02_pendingRamps->length = 0;
    this->line_02_startValue = this->line_02_output;
    this->line_02_slope = 0;
    this->line_02__time = 0;
    this->line_02_time_set(0);
}

number line_02_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_02_grain_set(number v) {
    v = this->line_02_grain_constrain(v);
    this->line_02_grain = v;

    if ((bool)(!(bool)(this->line_02_isFinished(this->line_02_startValue)))) {
        this->getEngine()->flushClockEvents(this, 1634454928, false);;
        this->line_02_scheduleNext();
    }
}

void line_02_end_bang() {}

void line_02_target_set(const list& v) {
    this->line_02_target = jsCreateListCopy(v);
    this->line_02_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_02__time = this->line_02_time;
        this->line_02_time_set(0);

        if ((bool)(this->line_02__time)) {
            this->line_02_startRamp(v[0], this->line_02__time);
        } else {
            this->line_02_output_set(v[0]);
            this->line_02_startValue = v[0];
            this->line_02_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_02_time_set(0);
        this->line_02__time = (v[1] < 0 ? 0 : v[1]);
        this->line_02_startRamp(v[0], this->line_02__time);
    } else if (v->length == 3) {
        this->line_02_time_set(0);
        this->line_02_grain_set(v[2]);
        this->line_02__time = (v[1] < 0 ? 0 : v[1]);
        this->line_02_startRamp(v[0], this->line_02__time);
    } else {
        this->line_02_time_set(0);
        this->line_02_pendingRamps = jsCreateListCopy(v);
        this->line_02_startPendingRamp();
    }
}

void message_04_out_set(const list& v) {
    this->line_02_target_set(v);
}

void message_04_trigger_bang() {
    this->message_04_out_set(this->message_04_set);
}

void groove_01_rate_bang_bang() {
    this->groove_01_changeIncomingInSamples = this->sampleOffsetIntoNextAudioBuffer + 1;
    this->groove_01_incomingChange = 1;
}

void button_02_output_bang() {
    this->message_17_trigger_bang();
    this->message_15_trigger_bang();
    this->message_12_trigger_bang();
    this->message_10_trigger_bang();
    this->message_09_trigger_bang();
    this->message_08_trigger_bang();
    this->message_06_trigger_bang();
    this->message_04_trigger_bang();
    this->groove_01_rate_bang_bang();
}

void button_02_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-57"), this->_currentTime);;
    this->button_02_output_bang();
}

number param_02_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

number line_11_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_11_time_set(number v) {
    v = this->line_11_time_constrain(v);
    this->line_11_time = v;
}

void gaintilde_02_outval_set(number ) {}

void gaintilde_02_value_set(number v) {
    this->gaintilde_02_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-124"), v, this->_currentTime);
    this->gaintilde_02_outval_set(value);
}

void gaintilde_02_input_number_set(number v) {
    this->gaintilde_02_input_number = v;

    this->gaintilde_02_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void gaintilde_01_outval_set(number ) {}

void gaintilde_01_value_set(number v) {
    this->gaintilde_01_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-122"), v, this->_currentTime);
    this->gaintilde_01_outval_set(value);
}

void gaintilde_01_input_number_set(number v) {
    this->gaintilde_01_input_number = v;

    this->gaintilde_01_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void line_11_output_set(number v) {
    this->line_11_output = v;
    this->gaintilde_02_input_number_set(v);
    this->gaintilde_01_input_number_set(v);
}

void line_11_stop_bang() {
    this->getEngine()->flushClockEvents(this, 1220262738, false);;
    this->line_11_pendingRamps->length = 0;
    this->line_11_startValue = this->line_11_output;
    this->line_11_slope = 0;
    this->line_11__time = 0;
    this->line_11_time_set(0);
}

number line_11_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_11_grain_set(number v) {
    v = this->line_11_grain_constrain(v);
    this->line_11_grain = v;

    if ((bool)(!(bool)(this->line_11_isFinished(this->line_11_startValue)))) {
        this->getEngine()->flushClockEvents(this, 1220262738, false);;
        this->line_11_scheduleNext();
    }
}

void line_11_end_bang() {}

void line_11_target_set(const list& v) {
    this->line_11_target = jsCreateListCopy(v);
    this->line_11_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_11__time = this->line_11_time;
        this->line_11_time_set(0);

        if ((bool)(this->line_11__time)) {
            this->line_11_startRamp(v[0], this->line_11__time);
        } else {
            this->line_11_output_set(v[0]);
            this->line_11_startValue = v[0];
            this->line_11_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_11_time_set(0);
        this->line_11__time = (v[1] < 0 ? 0 : v[1]);
        this->line_11_startRamp(v[0], this->line_11__time);
    } else if (v->length == 3) {
        this->line_11_time_set(0);
        this->line_11_grain_set(v[2]);
        this->line_11__time = (v[1] < 0 ? 0 : v[1]);
        this->line_11_startRamp(v[0], this->line_11__time);
    } else {
        this->line_11_time_set(0);
        this->line_11_pendingRamps = jsCreateListCopy(v);
        this->line_11_startPendingRamp();
    }
}

void message_13_out_set(const list& v) {
    this->line_11_target_set(v);
}

void message_13_trigger_bang() {
    this->message_13_out_set(this->message_13_set);
}

number line_08_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_08_time_set(number v) {
    v = this->line_08_time_constrain(v);
    this->line_08_time = v;
}

void gaintilde_04_outval_set(number ) {}

void gaintilde_04_value_set(number v) {
    this->gaintilde_04_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-116"), v, this->_currentTime);
    this->gaintilde_04_outval_set(value);
}

void gaintilde_04_input_number_set(number v) {
    this->gaintilde_04_input_number = v;

    this->gaintilde_04_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void gaintilde_03_outval_set(number ) {}

void gaintilde_03_value_set(number v) {
    this->gaintilde_03_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-120"), v, this->_currentTime);
    this->gaintilde_03_outval_set(value);
}

void gaintilde_03_input_number_set(number v) {
    this->gaintilde_03_input_number = v;

    this->gaintilde_03_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void line_08_output_set(number v) {
    this->line_08_output = v;
    this->gaintilde_04_input_number_set(v);
    this->gaintilde_03_input_number_set(v);
}

void line_08_stop_bang() {
    this->getEngine()->flushClockEvents(this, 1592269969, false);;
    this->line_08_pendingRamps->length = 0;
    this->line_08_startValue = this->line_08_output;
    this->line_08_slope = 0;
    this->line_08__time = 0;
    this->line_08_time_set(0);
}

number line_08_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_08_grain_set(number v) {
    v = this->line_08_grain_constrain(v);
    this->line_08_grain = v;

    if ((bool)(!(bool)(this->line_08_isFinished(this->line_08_startValue)))) {
        this->getEngine()->flushClockEvents(this, 1592269969, false);;
        this->line_08_scheduleNext();
    }
}

void line_08_end_bang() {}

void line_08_target_set(const list& v) {
    this->line_08_target = jsCreateListCopy(v);
    this->line_08_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_08__time = this->line_08_time;
        this->line_08_time_set(0);

        if ((bool)(this->line_08__time)) {
            this->line_08_startRamp(v[0], this->line_08__time);
        } else {
            this->line_08_output_set(v[0]);
            this->line_08_startValue = v[0];
            this->line_08_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_08_time_set(0);
        this->line_08__time = (v[1] < 0 ? 0 : v[1]);
        this->line_08_startRamp(v[0], this->line_08__time);
    } else if (v->length == 3) {
        this->line_08_time_set(0);
        this->line_08_grain_set(v[2]);
        this->line_08__time = (v[1] < 0 ? 0 : v[1]);
        this->line_08_startRamp(v[0], this->line_08__time);
    } else {
        this->line_08_time_set(0);
        this->line_08_pendingRamps = jsCreateListCopy(v);
        this->line_08_startPendingRamp();
    }
}

void message_11_out_set(const list& v) {
    this->line_08_target_set(v);
}

void message_11_trigger_bang() {
    this->message_11_out_set(this->message_11_set);
}

void select_02_match1_bang() {
    this->message_13_trigger_bang();
    this->message_11_trigger_bang();
}

void select_02_nomatch_number_set(number ) {}

void select_02_input_number_set(number v) {
    if (v == this->select_02_test1)
        this->select_02_match1_bang();
    else
        this->select_02_nomatch_number_set(v);
}

number param_03_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

number line_16_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_16_time_set(number v) {
    v = this->line_16_time_constrain(v);
    this->line_16_time = v;
}

void gaintilde_06_outval_set(number ) {}

void gaintilde_06_value_set(number v) {
    this->gaintilde_06_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-227"), v, this->_currentTime);
    this->gaintilde_06_outval_set(value);
}

void gaintilde_06_input_number_set(number v) {
    this->gaintilde_06_input_number = v;

    this->gaintilde_06_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void gaintilde_05_outval_set(number ) {}

void gaintilde_05_value_set(number v) {
    this->gaintilde_05_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-228"), v, this->_currentTime);
    this->gaintilde_05_outval_set(value);
}

void gaintilde_05_input_number_set(number v) {
    this->gaintilde_05_input_number = v;

    this->gaintilde_05_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void line_16_output_set(number v) {
    this->line_16_output = v;
    this->gaintilde_06_input_number_set(v);
    this->gaintilde_05_input_number_set(v);
}

void line_16_stop_bang() {
    this->getEngine()->flushClockEvents(this, 848255507, false);;
    this->line_16_pendingRamps->length = 0;
    this->line_16_startValue = this->line_16_output;
    this->line_16_slope = 0;
    this->line_16__time = 0;
    this->line_16_time_set(0);
}

number line_16_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_16_grain_set(number v) {
    v = this->line_16_grain_constrain(v);
    this->line_16_grain = v;

    if ((bool)(!(bool)(this->line_16_isFinished(this->line_16_startValue)))) {
        this->getEngine()->flushClockEvents(this, 848255507, false);;
        this->line_16_scheduleNext();
    }
}

void line_16_end_bang() {}

void line_16_target_set(const list& v) {
    this->line_16_target = jsCreateListCopy(v);
    this->line_16_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_16__time = this->line_16_time;
        this->line_16_time_set(0);

        if ((bool)(this->line_16__time)) {
            this->line_16_startRamp(v[0], this->line_16__time);
        } else {
            this->line_16_output_set(v[0]);
            this->line_16_startValue = v[0];
            this->line_16_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_16_time_set(0);
        this->line_16__time = (v[1] < 0 ? 0 : v[1]);
        this->line_16_startRamp(v[0], this->line_16__time);
    } else if (v->length == 3) {
        this->line_16_time_set(0);
        this->line_16_grain_set(v[2]);
        this->line_16__time = (v[1] < 0 ? 0 : v[1]);
        this->line_16_startRamp(v[0], this->line_16__time);
    } else {
        this->line_16_time_set(0);
        this->line_16_pendingRamps = jsCreateListCopy(v);
        this->line_16_startPendingRamp();
    }
}

void message_20_out_set(const list& v) {
    this->line_16_target_set(v);
}

void message_20_trigger_bang() {
    this->message_20_out_set(this->message_20_set);
}

void message_18_out_set(const list& v) {
    this->line_08_target_set(v);
}

void message_18_trigger_bang() {
    this->message_18_out_set(this->message_18_set);
}

void select_03_match1_bang() {
    this->message_20_trigger_bang();
    this->message_18_trigger_bang();
}

void select_03_nomatch_number_set(number ) {}

void select_03_input_number_set(number v) {
    if (v == this->select_03_test1)
        this->select_03_match1_bang();
    else
        this->select_03_nomatch_number_set(v);
}

number param_04_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void pan_tilde_05_pos_set(number v) {
    this->pan_tilde_05_pos = v;
}

void numberobj_16_output_set(number v) {
    this->pan_tilde_05_pos_set(v);
}

void numberobj_16_value_set(number v) {
    this->numberobj_16_value_setter(v);
    v = this->numberobj_16_value;
    number localvalue = v;

    if (this->numberobj_16_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-236"), localvalue, this->_currentTime);
    this->numberobj_16_output_set(localvalue);
}

void groove_01_rate_auto_set(number v) {
    this->groove_01_rate_auto = v;
}

void toggle_01_output_set(number v) {
    this->groove_01_rate_auto_set(v);
}

void toggle_01_value_number_set(number v) {
    this->toggle_01_value_number_setter(v);
    v = this->toggle_01_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-103"), v, this->_currentTime);
    this->toggle_01_output_set(v);
}

void toggle_01_value_bang_bang() {
    number val = (this->toggle_01_value_number == 1 ? 0 : 1);
    this->toggle_01_value_number_set(val);
}

void loadbang_01_output_bang() {
    this->toggle_01_value_bang_bang();
}

void message_01_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_02_input_number_set(converted);
    }

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_01_input_number_set(converted);
    }
}

void message_01_trigger_bang() {
    this->message_01_out_set(this->message_01_set);
}

void loadbang_02_output_bang() {
    this->message_01_trigger_bang();
}

void message_01_set_set(const list& v) {
    this->message_01_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-126"), v, this->_currentTime);
}

void toggle_05_output_set(number ) {}

void toggle_05_value_number_set(number v) {
    this->toggle_05_value_number_setter(v);
    v = this->toggle_05_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-232"), v, this->_currentTime);
    this->toggle_05_output_set(v);
}

void toggle_05_value_bang_bang() {
    number val = (this->toggle_05_value_number == 1 ? 0 : 1);
    this->toggle_05_value_number_set(val);
}

void message_03_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_04_input_number_set(converted);
    }

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_03_input_number_set(converted);
    }
}

void message_03_trigger_bang() {
    this->message_03_out_set(this->message_03_set);
}

void groove_02_rate_auto_set(number v) {
    this->groove_02_rate_auto = v;
}

void toggle_02_output_set(number v) {
    this->groove_02_rate_auto_set(v);
}

void toggle_02_value_number_set(number v) {
    this->toggle_02_value_number_setter(v);
    v = this->toggle_02_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-96"), v, this->_currentTime);
    this->toggle_02_output_set(v);
}

void toggle_02_value_bang_bang() {
    number val = (this->toggle_02_value_number == 1 ? 0 : 1);
    this->toggle_02_value_number_set(val);
}

void loadbang_03_output_bang() {
    this->toggle_05_value_bang_bang();
    this->message_03_trigger_bang();
    this->toggle_02_value_bang_bang();
}

number line_14_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_14_time_set(number v) {
    v = this->line_14_time_constrain(v);
    this->line_14_time = v;
}

void message_19_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_14_time_set(converted);
    }
}

void message_19_trigger_bang() {
    this->message_19_out_set(this->message_19_set);
}

void pan_tilde_06_pos_set(number v) {
    this->pan_tilde_06_pos = v;
}

void numberobj_13_output_set(number v) {
    this->pan_tilde_06_pos_set(v);
}

void numberobj_13_value_set(number v) {
    this->numberobj_13_value_setter(v);
    v = this->numberobj_13_value;
    number localvalue = v;

    if (this->numberobj_13_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-262"), localvalue, this->_currentTime);
    this->numberobj_13_output_set(localvalue);
}

void line_14_output_set(number v) {
    this->line_14_output = v;
    this->numberobj_13_value_set(v);
}

void line_14_stop_bang() {
    this->getEngine()->flushClockEvents(this, 1964277200, false);;
    this->line_14_pendingRamps->length = 0;
    this->line_14_startValue = this->line_14_output;
    this->line_14_slope = 0;
    this->line_14__time = 0;
    this->line_14_time_set(0);
}

number line_14_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_14_grain_set(number v) {
    v = this->line_14_grain_constrain(v);
    this->line_14_grain = v;

    if ((bool)(!(bool)(this->line_14_isFinished(this->line_14_startValue)))) {
        this->getEngine()->flushClockEvents(this, 1964277200, false);;
        this->line_14_scheduleNext();
    }
}

void random_08_trigger_bang() {
    if (this->stackprotect_check())
        return;

    this->random_08_result_set(rnbo_abs(this->random_08_nz_next()) * this->random_08_limit);
}

void button_15_output_bang() {
    this->message_19_trigger_bang();
    this->random_08_trigger_bang();
}

void button_15_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-260"), this->_currentTime);;
    this->button_15_output_bang();
}

void line_14_end_bang() {
    this->button_15_bangval_bang();
}

void line_14_target_set(const list& v) {
    this->line_14_target = jsCreateListCopy(v);
    this->line_14_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_14__time = this->line_14_time;
        this->line_14_time_set(0);

        if ((bool)(this->line_14__time)) {
            this->line_14_startRamp(v[0], this->line_14__time);
        } else {
            this->line_14_output_set(v[0]);
            this->line_14_startValue = v[0];
            this->line_14_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_14_time_set(0);
        this->line_14__time = (v[1] < 0 ? 0 : v[1]);
        this->line_14_startRamp(v[0], this->line_14__time);
    } else if (v->length == 3) {
        this->line_14_time_set(0);
        this->line_14_grain_set(v[2]);
        this->line_14__time = (v[1] < 0 ? 0 : v[1]);
        this->line_14_startRamp(v[0], this->line_14__time);
    } else {
        this->line_14_time_set(0);
        this->line_14_pendingRamps = jsCreateListCopy(v);
        this->line_14_startPendingRamp();
    }
}

void random_08_result_set(number v) {
    {
        list converted = {v};
        this->line_14_target_set(converted);
    }
}

void button_13_output_bang() {
    this->message_19_trigger_bang();
    this->random_08_trigger_bang();
}

void button_13_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-258"), this->_currentTime);;
    this->button_13_output_bang();
}

number line_09_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_09_time_set(number v) {
    v = this->line_09_time_constrain(v);
    this->line_09_time = v;
}

void message_14_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_09_time_set(converted);
    }
}

void message_14_trigger_bang() {
    this->message_14_out_set(this->message_14_set);
}

void pan_tilde_04_pos_set(number v) {
    this->pan_tilde_04_pos = v;
}

void numberobj_09_output_set(number v) {
    this->pan_tilde_04_pos_set(v);
}

void numberobj_09_value_set(number v) {
    this->numberobj_09_value_setter(v);
    v = this->numberobj_09_value;
    number localvalue = v;

    if (this->numberobj_09_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-253"), localvalue, this->_currentTime);
    this->numberobj_09_output_set(localvalue);
}

void line_09_output_set(number v) {
    this->line_09_output = v;
    this->numberobj_09_value_set(v);
}

void line_09_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1958682865, false);;
    this->line_09_pendingRamps->length = 0;
    this->line_09_startValue = this->line_09_output;
    this->line_09_slope = 0;
    this->line_09__time = 0;
    this->line_09_time_set(0);
}

number line_09_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_09_grain_set(number v) {
    v = this->line_09_grain_constrain(v);
    this->line_09_grain = v;

    if ((bool)(!(bool)(this->line_09_isFinished(this->line_09_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1958682865, false);;
        this->line_09_scheduleNext();
    }
}

void random_05_trigger_bang() {
    if (this->stackprotect_check())
        return;

    this->random_05_result_set(rnbo_abs(this->random_05_nz_next()) * this->random_05_limit);
}

void button_11_output_bang() {
    this->message_14_trigger_bang();
    this->random_05_trigger_bang();
}

void button_11_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-251"), this->_currentTime);;
    this->button_11_output_bang();
}

void line_09_end_bang() {
    this->button_11_bangval_bang();
}

void line_09_target_set(const list& v) {
    this->line_09_target = jsCreateListCopy(v);
    this->line_09_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_09__time = this->line_09_time;
        this->line_09_time_set(0);

        if ((bool)(this->line_09__time)) {
            this->line_09_startRamp(v[0], this->line_09__time);
        } else {
            this->line_09_output_set(v[0]);
            this->line_09_startValue = v[0];
            this->line_09_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_09_time_set(0);
        this->line_09__time = (v[1] < 0 ? 0 : v[1]);
        this->line_09_startRamp(v[0], this->line_09__time);
    } else if (v->length == 3) {
        this->line_09_time_set(0);
        this->line_09_grain_set(v[2]);
        this->line_09__time = (v[1] < 0 ? 0 : v[1]);
        this->line_09_startRamp(v[0], this->line_09__time);
    } else {
        this->line_09_time_set(0);
        this->line_09_pendingRamps = jsCreateListCopy(v);
        this->line_09_startPendingRamp();
    }
}

void random_05_result_set(number v) {
    {
        list converted = {v};
        this->line_09_target_set(converted);
    }
}

void button_10_output_bang() {
    this->message_14_trigger_bang();
    this->random_05_trigger_bang();
}

void button_10_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-249"), this->_currentTime);;
    this->button_10_output_bang();
}

number line_03_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_03_time_set(number v) {
    v = this->line_03_time_constrain(v);
    this->line_03_time = v;
}

void message_07_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_03_time_set(converted);
    }
}

void message_07_trigger_bang() {
    this->message_07_out_set(this->message_07_set);
}

void pan_tilde_03_pos_set(number v) {
    this->pan_tilde_03_pos = v;
}

void numberobj_04_output_set(number v) {
    this->pan_tilde_03_pos_set(v);
}

void numberobj_04_value_set(number v) {
    this->numberobj_04_value_setter(v);
    v = this->numberobj_04_value;
    number localvalue = v;

    if (this->numberobj_04_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-244"), localvalue, this->_currentTime);
    this->numberobj_04_output_set(localvalue);
}

void line_03_output_set(number v) {
    this->line_03_output = v;
    this->numberobj_04_value_set(v);
}

void line_03_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1586675634, false);;
    this->line_03_pendingRamps->length = 0;
    this->line_03_startValue = this->line_03_output;
    this->line_03_slope = 0;
    this->line_03__time = 0;
    this->line_03_time_set(0);
}

number line_03_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_03_grain_set(number v) {
    v = this->line_03_grain_constrain(v);
    this->line_03_grain = v;

    if ((bool)(!(bool)(this->line_03_isFinished(this->line_03_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1586675634, false);;
        this->line_03_scheduleNext();
    }
}

void random_02_trigger_bang() {
    if (this->stackprotect_check())
        return;

    this->random_02_result_set(rnbo_abs(this->random_02_nz_next()) * this->random_02_limit);
}

void button_08_output_bang() {
    this->message_07_trigger_bang();
    this->random_02_trigger_bang();
}

void button_08_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-242"), this->_currentTime);;
    this->button_08_output_bang();
}

void line_03_end_bang() {
    this->button_08_bangval_bang();
}

void line_03_target_set(const list& v) {
    this->line_03_target = jsCreateListCopy(v);
    this->line_03_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_03__time = this->line_03_time;
        this->line_03_time_set(0);

        if ((bool)(this->line_03__time)) {
            this->line_03_startRamp(v[0], this->line_03__time);
        } else {
            this->line_03_output_set(v[0]);
            this->line_03_startValue = v[0];
            this->line_03_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_03_time_set(0);
        this->line_03__time = (v[1] < 0 ? 0 : v[1]);
        this->line_03_startRamp(v[0], this->line_03__time);
    } else if (v->length == 3) {
        this->line_03_time_set(0);
        this->line_03_grain_set(v[2]);
        this->line_03__time = (v[1] < 0 ? 0 : v[1]);
        this->line_03_startRamp(v[0], this->line_03__time);
    } else {
        this->line_03_time_set(0);
        this->line_03_pendingRamps = jsCreateListCopy(v);
        this->line_03_startPendingRamp();
    }
}

void random_02_result_set(number v) {
    {
        list converted = {v};
        this->line_03_target_set(converted);
    }
}

void button_06_output_bang() {
    this->message_07_trigger_bang();
    this->random_02_trigger_bang();
}

void button_06_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-240"), this->_currentTime);;
    this->button_06_output_bang();
}

number line_01_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_01_time_set(number v) {
    v = this->line_01_time_constrain(v);
    this->line_01_time = v;
}

void message_02_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_01_time_set(converted);
    }
}

void message_02_trigger_bang() {
    this->message_02_out_set(this->message_02_set);
}

void pan_tilde_01_pos_set(number v) {
    this->pan_tilde_01_pos = v;
}

void numberobj_01_output_set(number v) {
    this->pan_tilde_01_pos_set(v);
}

void numberobj_01_value_set(number v) {
    this->numberobj_01_value_setter(v);
    v = this->numberobj_01_value;
    number localvalue = v;

    if (this->numberobj_01_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-214"), localvalue, this->_currentTime);
    this->numberobj_01_output_set(localvalue);
}

void line_01_output_set(number v) {
    this->line_01_output = v;
    this->numberobj_01_value_set(v);
}

void line_01_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1214668403, false);;
    this->line_01_pendingRamps->length = 0;
    this->line_01_startValue = this->line_01_output;
    this->line_01_slope = 0;
    this->line_01__time = 0;
    this->line_01_time_set(0);
}

number line_01_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_01_grain_set(number v) {
    v = this->line_01_grain_constrain(v);
    this->line_01_grain = v;

    if ((bool)(!(bool)(this->line_01_isFinished(this->line_01_startValue)))) {
        this->getEngine()->flushClockEvents(this, -1214668403, false);;
        this->line_01_scheduleNext();
    }
}

void random_01_trigger_bang() {
    if (this->stackprotect_check())
        return;

    this->random_01_result_set(rnbo_abs(this->random_01_nz_next()) * this->random_01_limit);
}

void button_05_output_bang() {
    this->message_02_trigger_bang();
    this->random_01_trigger_bang();
}

void button_05_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-218"), this->_currentTime);;
    this->button_05_output_bang();
}

void line_01_end_bang() {
    this->button_05_bangval_bang();
}

void line_01_target_set(const list& v) {
    this->line_01_target = jsCreateListCopy(v);
    this->line_01_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_01__time = this->line_01_time;
        this->line_01_time_set(0);

        if ((bool)(this->line_01__time)) {
            this->line_01_startRamp(v[0], this->line_01__time);
        } else {
            this->line_01_output_set(v[0]);
            this->line_01_startValue = v[0];
            this->line_01_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_01_time_set(0);
        this->line_01__time = (v[1] < 0 ? 0 : v[1]);
        this->line_01_startRamp(v[0], this->line_01__time);
    } else if (v->length == 3) {
        this->line_01_time_set(0);
        this->line_01_grain_set(v[2]);
        this->line_01__time = (v[1] < 0 ? 0 : v[1]);
        this->line_01_startRamp(v[0], this->line_01__time);
    } else {
        this->line_01_time_set(0);
        this->line_01_pendingRamps = jsCreateListCopy(v);
        this->line_01_startPendingRamp();
    }
}

void random_01_result_set(number v) {
    {
        list converted = {v};
        this->line_01_target_set(converted);
    }
}

void button_03_output_bang() {
    this->message_02_trigger_bang();
    this->random_01_trigger_bang();
}

void button_03_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-233"), this->_currentTime);;
    this->button_03_output_bang();
}

void loadbang_04_output_bang() {
    this->button_13_bangval_bang();
    this->button_10_bangval_bang();
    this->button_06_bangval_bang();
    this->button_03_bangval_bang();
}

void message_02_set_set(const list& v) {
    this->message_02_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-216"), v, this->_currentTime);
}

void message_03_set_set(const list& v) {
    this->message_03_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-56"), v, this->_currentTime);
}

void groove_03_rate_auto_set(number v) {
    this->groove_03_rate_auto = v;
}

void toggle_03_output_set(number v) {
    this->groove_03_rate_auto_set(v);
}

void toggle_03_value_number_set(number v) {
    this->toggle_03_value_number_setter(v);
    v = this->toggle_03_value_number;
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("toggle_obj-223"), v, this->_currentTime);
    this->toggle_03_output_set(v);
}

void toggle_03_value_bang_bang() {
    number val = (this->toggle_03_value_number == 1 ? 0 : 1);
    this->toggle_03_value_number_set(val);
}

void loadbang_05_output_bang() {
    this->toggle_03_value_bang_bang();
}

void numberobj_02_output_set(number ) {}

void numberobj_02_value_set(number v) {
    this->numberobj_02_value_setter(v);
    v = this->numberobj_02_value;
    number localvalue = v;

    if (this->numberobj_02_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-41"), localvalue, this->_currentTime);
    this->numberobj_02_output_set(localvalue);
}

void numberobj_08_output_set(number ) {}

void numberobj_08_value_set(number v) {
    this->numberobj_08_value_setter(v);
    v = this->numberobj_08_value;
    number localvalue = v;

    if (this->numberobj_08_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-90"), localvalue, this->_currentTime);
    this->numberobj_08_output_set(localvalue);
}

void multibuffertilde_01_current_set(number v) {
    this->multibuffertilde_01_current = v;
    updateMultiRef(this, this->multibuffertilde_01_buffer, v);
}

void expr_02_out1_set(number v) {
    this->expr_02_out1 = v;
    this->multibuffertilde_01_current_set(this->expr_02_out1);
}

void expr_02_in1_set(number in1) {
    this->expr_02_in1 = in1;
    this->expr_02_out1_set(this->expr_02_in1 - this->expr_02_in2);//#map:-_obj-93:1
}

void expr_04_out1_set(number v) {
    this->expr_04_out1 = v;
    this->numberobj_08_value_set(this->expr_04_out1);
    this->expr_02_in1_set(this->expr_04_out1);
}

void expr_04_in1_set(number in1) {
    this->expr_04_in1 = in1;

    this->expr_04_out1_set(
        (this->expr_04_in2 == 0 ? 0 : rnbo_fround((this->expr_04_in2 == 0. ? 0. : this->expr_04_in1 * 1 / this->expr_04_in2)) * this->expr_04_in2)
    );//#map:round_obj-79:1
}

void expr_03_out1_set(number v) {
    this->expr_03_out1 = v;
    this->expr_04_in1_set(this->expr_03_out1);
}

void expr_03_in1_set(number in1) {
    this->expr_03_in1 = in1;
    this->expr_03_out1_set(this->expr_03_in1 + this->expr_03_in2);//#map:+_obj-91:1
}

void random_04_result_set(number v) {
    this->expr_03_in1_set(v);
}

void random_04_trigger_bang() {
    this->random_04_result_set(rnbo_abs(this->random_04_nz_next()) * this->random_04_limit);
}

void numberobj_05_output_set(number ) {}

void numberobj_05_value_set(number v) {
    this->numberobj_05_value_setter(v);
    v = this->numberobj_05_value;
    number localvalue = v;

    if (this->numberobj_05_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-212"), localvalue, this->_currentTime);
    this->numberobj_05_output_set(localvalue);
}

void pan_tilde_02_pos_set(number v) {
    this->pan_tilde_02_pos = v;
}

void random_03_result_set(number v) {
    this->numberobj_05_value_set(v);
    this->pan_tilde_02_pos_set(v);
}

void random_03_trigger_bang() {
    this->random_03_result_set(rnbo_abs(this->random_03_nz_next()) * this->random_03_limit);
}

void groove_03_rate_bang_bang() {
    this->groove_03_changeIncomingInSamples = this->sampleOffsetIntoNextAudioBuffer + 1;
    this->groove_03_incomingChange = 1;
}

void button_07_output_bang() {
    this->random_04_trigger_bang();
    this->random_03_trigger_bang();
    this->groove_03_rate_bang_bang();
}

void button_07_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-217"), this->_currentTime);;
    this->button_07_output_bang();
}

void select_01_match1_bang() {
    this->button_07_bangval_bang();
}

void select_01_match2_bang() {
    this->button_07_bangval_bang();
}

void select_01_match3_bang() {
    this->button_07_bangval_bang();
}

void select_01_match4_bang() {
    this->button_07_bangval_bang();
}

void select_01_match5_bang() {
    this->button_07_bangval_bang();
}

void select_01_nomatch_number_set(number ) {}

void select_01_input_number_set(number v) {
    if (v == this->select_01_test1)
        this->select_01_match1_bang();
    else if (v == this->select_01_test2)
        this->select_01_match2_bang();
    else if (v == this->select_01_test3)
        this->select_01_match3_bang();
    else if (v == this->select_01_test4)
        this->select_01_match4_bang();
    else if (v == this->select_01_test5)
        this->select_01_match5_bang();
    else
        this->select_01_nomatch_number_set(v);
}

void counter_01_output_set(number v) {
    this->counter_01_output = v;
    this->select_01_input_number_set(v);
    this->numberobj_02_value_set(v);
}

void counter_01_overflow_bang_bang() {}

number counter_01_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_01_overflow_number_set(number v) {
    v = this->counter_01_overflow_number_constrain(v);
    this->counter_01_overflow_number = v;
}

void counter_01_carry_set(number v) {
    this->counter_01_carry = v;
}

void counter_01_underflow_bang_bang() {}

number counter_01_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_01_underflow_number_set(number v) {
    v = this->counter_01_underflow_number_constrain(v);
    this->counter_01_underflow_number = v;
}

void counter_01_input_bang() {
    this->counter_01_output_set(this->counter_01_count);
    this->counter_01_inc();

    if (this->counter_01_count > this->counter_01_maximum) {
        if (this->counter_01_direction == 2) {
            this->counter_01_phase = !(bool)(this->counter_01_phase);
            this->counter_01_count = this->counter_01_maximum;
            this->counter_01_inc();
        } else
            this->counter_01_count = this->counter_01_minimum;

        if ((bool)(!(bool)(this->counter_01_overflow_number))) {
            if (this->counter_01_carryflag == 1)
                this->counter_01_overflow_bang_bang();
            else
                this->counter_01_overflow_number_set(1);
        }

        this->counter_01_carry_set(this->counter_01_carry + 1);
    } else if ((bool)(this->counter_01_overflow_number) && this->counter_01_carryflag == 0)
        this->counter_01_overflow_number_set(0);

    if (this->counter_01_count < this->counter_01_minimum) {
        if (this->counter_01_direction == 2) {
            this->counter_01_phase = !(bool)(this->counter_01_phase);
            this->counter_01_count = this->counter_01_minimum;
            this->counter_01_inc();
        } else
            this->counter_01_count = this->counter_01_maximum;

        if ((bool)(!(bool)(this->counter_01_underflow_number))) {
            if (this->counter_01_carryflag == 1)
                this->counter_01_underflow_bang_bang();
            else
                this->counter_01_underflow_number_set(1);
        }

        this->counter_01_carry_set(this->counter_01_carry + 1);
    } else if ((bool)(this->counter_01_underflow_number) && this->counter_01_carryflag == 0)
        this->counter_01_underflow_number_set(0);
}

void metro_01_tickout_bang() {
    this->counter_01_input_bang();
}

void message_04_set_set(const list& v) {
    this->message_04_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-5"), v, this->_currentTime);
}

void transport_01_tempo_set(number v) {
    this->transport_01_tempo = v;

    if (v >= 0) {
        this->getTopLevelPatcher()->globaltransport_setTempo(this->currenttime(), v, true);
    }
}

void message_05_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->transport_01_tempo_set(converted);
    }
}

void message_05_trigger_bang() {
    this->message_05_out_set(this->message_05_set);
}

void loadbang_06_output_bang() {
    this->message_05_trigger_bang();
}

void message_05_set_set(const list& v) {
    this->message_05_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-75"), v, this->_currentTime);
}

void message_06_set_set(const list& v) {
    this->message_06_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-10"), v, this->_currentTime);
}

void message_07_set_set(const list& v) {
    this->message_07_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-243"), v, this->_currentTime);
}

void message_08_set_set(const list& v) {
    this->message_08_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-12"), v, this->_currentTime);
}

void gaintilde_08_outval_set(number ) {}

void gaintilde_08_value_set(number v) {
    this->gaintilde_08_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-74"), v, this->_currentTime);
    this->gaintilde_08_outval_set(value);
}

void gaintilde_08_input_number_set(number v) {
    this->gaintilde_08_input_number = v;

    this->gaintilde_08_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void gaintilde_07_outval_set(number ) {}

void gaintilde_07_value_set(number v) {
    this->gaintilde_07_value = v;
    number value;
    value = this->scale(v, 0, 157, 0, 158 - 1, 1);
    this->getEngine()->sendNumMessage(TAG("valout"), TAG("gain~_obj-18"), v, this->_currentTime);
    this->gaintilde_07_outval_set(value);
}

void gaintilde_07_input_number_set(number v) {
    this->gaintilde_07_input_number = v;

    this->gaintilde_07_value_set(
        this->scale((v > 158 - 1 ? 158 - 1 : (v < 0 ? 0 : v)), 0, 158 - 1, 0, 157, 1)
    );
}

void message_16_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_08_input_number_set(converted);
    }

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gaintilde_07_input_number_set(converted);
    }
}

void message_16_trigger_bang() {
    this->message_16_out_set(this->message_16_set);
}

void loadbang_07_output_bang() {
    this->message_16_trigger_bang();
}

void message_09_set_set(const list& v) {
    this->message_09_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-17"), v, this->_currentTime);
}

void message_10_set_set(const list& v) {
    this->message_10_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-59"), v, this->_currentTime);
}

void message_11_set_set(const list& v) {
    this->message_11_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-129"), v, this->_currentTime);
}

void button_09_output_bang() {
    this->random_04_trigger_bang();
}

void button_09_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-84"), this->_currentTime);;
    this->button_09_output_bang();
}

void message_12_set_set(const list& v) {
    this->message_12_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-60"), v, this->_currentTime);
}

void message_13_set_set(const list& v) {
    this->message_13_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-135"), v, this->_currentTime);
}

void message_14_set_set(const list& v) {
    this->message_14_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-252"), v, this->_currentTime);
}

void message_15_set_set(const list& v) {
    this->message_15_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-62"), v, this->_currentTime);
}

void message_16_set_set(const list& v) {
    this->message_16_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-2"), v, this->_currentTime);
}

void message_17_set_set(const list& v) {
    this->message_17_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-64"), v, this->_currentTime);
}

void numberobj_11_output_set(number ) {}

void numberobj_11_value_set(number v) {
    this->numberobj_11_value_setter(v);
    v = this->numberobj_11_value;
    number localvalue = v;

    if (this->numberobj_11_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-82"), localvalue, this->_currentTime);
    this->numberobj_11_output_set(localvalue);
}

void button_14_output_bang() {}

void button_14_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-78"), this->_currentTime);;
    this->button_14_output_bang();
}

void counter_02_output_set(number v) {
    this->counter_02_output = v;
    this->numberobj_11_value_set(v);
}

void counter_02_overflow_bang_bang() {}

number counter_02_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_02_overflow_number_set(number v) {
    v = this->counter_02_overflow_number_constrain(v);
    this->counter_02_overflow_number = v;
}

void numberobj_18_output_set(number ) {}

void numberobj_18_value_set(number v) {
    this->numberobj_18_value_setter(v);
    v = this->numberobj_18_value;
    number localvalue = v;

    if (this->numberobj_18_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-44"), localvalue, this->_currentTime);
    this->numberobj_18_output_set(localvalue);
}

void switchobj_01_onoff_set(number v) {
    this->switchobj_01_onoff = v;
}

void expr_07_out1_set(number v) {
    this->expr_07_out1 = v;
    this->numberobj_18_value_set(this->expr_07_out1);
    this->switchobj_01_onoff_set(this->expr_07_out1);
}

void expr_07_in1_set(number in1) {
    this->expr_07_in1 = in1;
    this->expr_07_out1_set(this->expr_07_in1 + this->expr_07_in2);//#map:+_obj-119:1
}

void counter_03_output_set(number v) {
    this->counter_03_output = v;
    this->expr_07_in1_set(v);
}

void counter_03_overflow_bang_bang() {}

number counter_03_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_03_overflow_number_set(number v) {
    v = this->counter_03_overflow_number_constrain(v);
    this->counter_03_overflow_number = v;
}

void counter_03_carry_set(number v) {
    this->counter_03_carry = v;
}

void counter_03_underflow_bang_bang() {}

number counter_03_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_03_underflow_number_set(number v) {
    v = this->counter_03_underflow_number_constrain(v);
    this->counter_03_underflow_number = v;
}

void counter_03_input_bang() {
    this->counter_03_output_set(this->counter_03_count);
    this->counter_03_inc();

    if (this->counter_03_count > this->counter_03_maximum) {
        if (this->counter_03_direction == 2) {
            this->counter_03_phase = !(bool)(this->counter_03_phase);
            this->counter_03_count = this->counter_03_maximum;
            this->counter_03_inc();
        } else
            this->counter_03_count = this->counter_03_minimum;

        if ((bool)(!(bool)(this->counter_03_overflow_number))) {
            if (this->counter_03_carryflag == 1)
                this->counter_03_overflow_bang_bang();
            else
                this->counter_03_overflow_number_set(1);
        }

        this->counter_03_carry_set(this->counter_03_carry + 1);
    } else if ((bool)(this->counter_03_overflow_number) && this->counter_03_carryflag == 0)
        this->counter_03_overflow_number_set(0);

    if (this->counter_03_count < this->counter_03_minimum) {
        if (this->counter_03_direction == 2) {
            this->counter_03_phase = !(bool)(this->counter_03_phase);
            this->counter_03_count = this->counter_03_minimum;
            this->counter_03_inc();
        } else
            this->counter_03_count = this->counter_03_maximum;

        if ((bool)(!(bool)(this->counter_03_underflow_number))) {
            if (this->counter_03_carryflag == 1)
                this->counter_03_underflow_bang_bang();
            else
                this->counter_03_underflow_number_set(1);
        }

        this->counter_03_carry_set(this->counter_03_carry + 1);
    } else if ((bool)(this->counter_03_underflow_number) && this->counter_03_carryflag == 0)
        this->counter_03_underflow_number_set(0);
}

number line_15_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_15_time_set(number v) {
    v = this->line_15_time_constrain(v);
    this->line_15_time = v;
}

void message_21_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_15_time_set(converted);
    }
}

void message_21_trigger_bang() {
    this->message_21_out_set(this->message_21_set);
}

void numberobj_17_output_set(number ) {}

void numberobj_17_value_set(number v) {
    this->numberobj_17_value_setter(v);
    v = this->numberobj_17_value;
    number localvalue = v;

    if (this->numberobj_17_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-121"), localvalue, this->_currentTime);
    this->numberobj_17_output_set(localvalue);
}

void numberobj_15_output_set(number ) {}

void numberobj_15_value_set(number v) {
    this->numberobj_15_value_setter(v);
    v = this->numberobj_15_value;
    number localvalue = v;

    if (this->numberobj_15_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-94"), localvalue, this->_currentTime);
    this->numberobj_15_output_set(localvalue);
}

void line_15_output_set(number v) {
    this->line_15_output = v;
    this->numberobj_15_value_set(v);
    this->slider_02_input_set(v);
}

void line_15_stop_bang() {
    this->getEngine()->flushClockEvents(this, 146426004, false);;
    this->line_15_pendingRamps->length = 0;
    this->line_15_startValue = this->line_15_output;
    this->line_15_slope = 0;
    this->line_15__time = 0;
    this->line_15_time_set(0);
}

number line_15_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_15_grain_set(number v) {
    v = this->line_15_grain_constrain(v);
    this->line_15_grain = v;

    if ((bool)(!(bool)(this->line_15_isFinished(this->line_15_startValue)))) {
        this->getEngine()->flushClockEvents(this, 146426004, false);;
        this->line_15_scheduleNext();
    }
}

void line_15_end_bang() {}

void line_15_target_set(const list& v) {
    this->line_15_target = jsCreateListCopy(v);
    this->line_15_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_15__time = this->line_15_time;
        this->line_15_time_set(0);

        if ((bool)(this->line_15__time)) {
            this->line_15_startRamp(v[0], this->line_15__time);
        } else {
            this->line_15_output_set(v[0]);
            this->line_15_startValue = v[0];
            this->line_15_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_15_time_set(0);
        this->line_15__time = (v[1] < 0 ? 0 : v[1]);
        this->line_15_startRamp(v[0], this->line_15__time);
    } else if (v->length == 3) {
        this->line_15_time_set(0);
        this->line_15_grain_set(v[2]);
        this->line_15__time = (v[1] < 0 ? 0 : v[1]);
        this->line_15_startRamp(v[0], this->line_15__time);
    } else {
        this->line_15_time_set(0);
        this->line_15_pendingRamps = jsCreateListCopy(v);
        this->line_15_startPendingRamp();
    }
}

void switchobj_01_out_number_set(number v) {
    {
        list converted = {v};
        this->line_15_target_set(converted);
    }
}

void switchobj_01_in2_number_set(number v) {
    if (this->switchobj_01_onoff == 2) {
        this->switchobj_01_out_number_set(v);
    }
}

void expr_06_out1_set(number v) {
    this->expr_06_out1 = v;
    this->numberobj_17_value_set(this->expr_06_out1);
    this->switchobj_01_in2_number_set(this->expr_06_out1);
}

void expr_06_in1_set(number in1) {
    this->expr_06_in1 = in1;
    this->expr_06_out1_set(this->expr_06_in1 + this->expr_06_in2);//#map:+_obj-88:1
}

void random_09_result_set(number v) {
    this->expr_06_in1_set(v);
}

void random_09_trigger_bang() {
    this->random_09_result_set(rnbo_abs(this->random_09_nz_next()) * this->random_09_limit);
}

void numberobj_12_output_set(number ) {}

void numberobj_12_value_set(number v) {
    this->numberobj_12_value_setter(v);
    v = this->numberobj_12_value;
    number localvalue = v;

    if (this->numberobj_12_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-100"), localvalue, this->_currentTime);
    this->numberobj_12_output_set(localvalue);
}

void switchobj_01_in1_number_set(number v) {
    if (this->switchobj_01_onoff == 1) {
        this->switchobj_01_out_number_set(v);
    }
}

void expr_05_out1_set(number v) {
    this->expr_05_out1 = v;
    this->numberobj_12_value_set(this->expr_05_out1);
    this->switchobj_01_in1_number_set(this->expr_05_out1);
}

void expr_05_in1_set(number in1) {
    this->expr_05_in1 = in1;
    this->expr_05_out1_set(this->expr_05_in1 + this->expr_05_in2);//#map:+_obj-86:1
}

void random_07_result_set(number v) {
    this->expr_05_in1_set(v);
}

void random_07_trigger_bang() {
    this->random_07_result_set(rnbo_abs(this->random_07_nz_next()) * this->random_07_limit);
}

void button_17_output_bang() {
    this->counter_03_input_bang();
    this->message_21_trigger_bang();
    this->random_09_trigger_bang();
    this->random_07_trigger_bang();
}

void button_17_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-98"), this->_currentTime);;
    this->button_17_output_bang();
}

void button_16_output_bang() {
    this->button_17_bangval_bang();
    this->random_06_trigger_bang();
}

void button_16_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-133"), this->_currentTime);;
    this->button_16_output_bang();
}

void counter_02_carry_set(number v) {
    this->counter_02_carry = v;
    this->button_16_bangval_bang();
}

void counter_02_underflow_bang_bang() {}

number counter_02_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_02_underflow_number_set(number v) {
    v = this->counter_02_underflow_number_constrain(v);
    this->counter_02_underflow_number = v;
}

void counter_02_input_bang() {
    this->counter_02_output_set(this->counter_02_count);
    this->counter_02_inc();

    if (this->counter_02_count > this->counter_02_maximum) {
        if (this->counter_02_direction == 2) {
            this->counter_02_phase = !(bool)(this->counter_02_phase);
            this->counter_02_count = this->counter_02_maximum;
            this->counter_02_inc();
        } else
            this->counter_02_count = this->counter_02_minimum;

        if ((bool)(!(bool)(this->counter_02_overflow_number))) {
            if (this->counter_02_carryflag == 1)
                this->counter_02_overflow_bang_bang();
            else
                this->counter_02_overflow_number_set(1);
        }

        this->counter_02_carry_set(this->counter_02_carry + 1);
    } else if ((bool)(this->counter_02_overflow_number) && this->counter_02_carryflag == 0)
        this->counter_02_overflow_number_set(0);

    if (this->counter_02_count < this->counter_02_minimum) {
        if (this->counter_02_direction == 2) {
            this->counter_02_phase = !(bool)(this->counter_02_phase);
            this->counter_02_count = this->counter_02_minimum;
            this->counter_02_inc();
        } else
            this->counter_02_count = this->counter_02_maximum;

        if ((bool)(!(bool)(this->counter_02_underflow_number))) {
            if (this->counter_02_carryflag == 1)
                this->counter_02_underflow_bang_bang();
            else
                this->counter_02_underflow_number_set(1);
        }

        this->counter_02_carry_set(this->counter_02_carry + 1);
    } else if ((bool)(this->counter_02_underflow_number) && this->counter_02_carryflag == 0)
        this->counter_02_underflow_number_set(0);
}

void metro_02_tickout_bang() {
    this->button_14_bangval_bang();
    this->counter_02_input_bang();
}

void message_18_set_set(const list& v) {
    this->message_18_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-211"), v, this->_currentTime);
}

void message_19_set_set(const list& v) {
    this->message_19_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-261"), v, this->_currentTime);
}

void message_20_set_set(const list& v) {
    this->message_20_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-219"), v, this->_currentTime);
}

void message_21_set_set(const list& v) {
    this->message_21_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-106"), v, this->_currentTime);
}

void delay_01_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1357044121, false);;
}

void delay_01_input_bang() {
    if ((bool)(!(bool)(this->delay_01_delayall)))
        this->delay_01_stop_bang();

    this->getEngine()->scheduleClockEvent(this, -1357044121, this->delay_01_time + this->_currentTime);;
}

void button_18_output_bang() {
    this->toggle_09_value_bang_bang();
    this->toggle_08_value_bang_bang();
    this->toggle_07_value_bang_bang();
    this->toggle_06_value_bang_bang();
}

void button_18_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-72"), this->_currentTime);;
    this->button_18_output_bang();
}

void loadbang_08_output_bang() {
    this->delay_01_input_bang();
    this->button_18_bangval_bang();
}

void numberobj_19_output_set(number ) {}

void numberobj_19_value_set(number v) {
    this->numberobj_19_value_setter(v);
    v = this->numberobj_19_value;
    number localvalue = v;

    if (this->numberobj_19_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-151"), localvalue, this->_currentTime);
    this->numberobj_19_output_set(localvalue);
}

void button_19_output_bang() {}

void button_19_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-153"), this->_currentTime);;
    this->button_19_output_bang();
}

void counter_04_output_set(number v) {
    this->counter_04_output = v;
    this->numberobj_19_value_set(v);
}

void counter_04_overflow_bang_bang() {}

number counter_04_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_04_overflow_number_set(number v) {
    v = this->counter_04_overflow_number_constrain(v);
    this->counter_04_overflow_number = v;
}

void numberobj_24_output_set(number ) {}

void numberobj_24_value_set(number v) {
    this->numberobj_24_value_setter(v);
    v = this->numberobj_24_value;
    number localvalue = v;

    if (this->numberobj_24_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-156"), localvalue, this->_currentTime);
    this->numberobj_24_output_set(localvalue);
}

void switchobj_02_onoff_set(number v) {
    this->switchobj_02_onoff = v;
}

void expr_10_out1_set(number v) {
    this->expr_10_out1 = v;
    this->numberobj_24_value_set(this->expr_10_out1);
    this->switchobj_02_onoff_set(this->expr_10_out1);
}

void expr_10_in1_set(number in1) {
    this->expr_10_in1 = in1;
    this->expr_10_out1_set(this->expr_10_in1 + this->expr_10_in2);//#map:+_obj-138:1
}

void counter_05_output_set(number v) {
    this->counter_05_output = v;
    this->expr_10_in1_set(v);
}

void counter_05_overflow_bang_bang() {}

number counter_05_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_05_overflow_number_set(number v) {
    v = this->counter_05_overflow_number_constrain(v);
    this->counter_05_overflow_number = v;
}

void counter_05_carry_set(number v) {
    this->counter_05_carry = v;
}

void counter_05_underflow_bang_bang() {}

number counter_05_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_05_underflow_number_set(number v) {
    v = this->counter_05_underflow_number_constrain(v);
    this->counter_05_underflow_number = v;
}

void counter_05_input_bang() {
    this->counter_05_output_set(this->counter_05_count);
    this->counter_05_inc();

    if (this->counter_05_count > this->counter_05_maximum) {
        if (this->counter_05_direction == 2) {
            this->counter_05_phase = !(bool)(this->counter_05_phase);
            this->counter_05_count = this->counter_05_maximum;
            this->counter_05_inc();
        } else
            this->counter_05_count = this->counter_05_minimum;

        if ((bool)(!(bool)(this->counter_05_overflow_number))) {
            if (this->counter_05_carryflag == 1)
                this->counter_05_overflow_bang_bang();
            else
                this->counter_05_overflow_number_set(1);
        }

        this->counter_05_carry_set(this->counter_05_carry + 1);
    } else if ((bool)(this->counter_05_overflow_number) && this->counter_05_carryflag == 0)
        this->counter_05_overflow_number_set(0);

    if (this->counter_05_count < this->counter_05_minimum) {
        if (this->counter_05_direction == 2) {
            this->counter_05_phase = !(bool)(this->counter_05_phase);
            this->counter_05_count = this->counter_05_minimum;
            this->counter_05_inc();
        } else
            this->counter_05_count = this->counter_05_maximum;

        if ((bool)(!(bool)(this->counter_05_underflow_number))) {
            if (this->counter_05_carryflag == 1)
                this->counter_05_underflow_bang_bang();
            else
                this->counter_05_underflow_number_set(1);
        }

        this->counter_05_carry_set(this->counter_05_carry + 1);
    } else if ((bool)(this->counter_05_underflow_number) && this->counter_05_carryflag == 0)
        this->counter_05_underflow_number_set(0);
}

number line_17_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_17_time_set(number v) {
    v = this->line_17_time_constrain(v);
    this->line_17_time = v;
}

void message_22_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_17_time_set(converted);
    }
}

void message_22_trigger_bang() {
    this->message_22_out_set(this->message_22_set);
}

void numberobj_23_output_set(number ) {}

void numberobj_23_value_set(number v) {
    this->numberobj_23_value_setter(v);
    v = this->numberobj_23_value;
    number localvalue = v;

    if (this->numberobj_23_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-137"), localvalue, this->_currentTime);
    this->numberobj_23_output_set(localvalue);
}

void numberobj_22_output_set(number ) {}

void numberobj_22_value_set(number v) {
    this->numberobj_22_value_setter(v);
    v = this->numberobj_22_value;
    number localvalue = v;

    if (this->numberobj_22_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-144"), localvalue, this->_currentTime);
    this->numberobj_22_output_set(localvalue);
}

void line_17_output_set(number v) {
    this->line_17_output = v;
    this->numberobj_22_value_set(v);
    this->slider_04_input_set(v);
}

void line_17_stop_bang() {
    this->getEngine()->flushClockEvents(this, -225581227, false);;
    this->line_17_pendingRamps->length = 0;
    this->line_17_startValue = this->line_17_output;
    this->line_17_slope = 0;
    this->line_17__time = 0;
    this->line_17_time_set(0);
}

number line_17_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_17_grain_set(number v) {
    v = this->line_17_grain_constrain(v);
    this->line_17_grain = v;

    if ((bool)(!(bool)(this->line_17_isFinished(this->line_17_startValue)))) {
        this->getEngine()->flushClockEvents(this, -225581227, false);;
        this->line_17_scheduleNext();
    }
}

void line_17_end_bang() {}

void line_17_target_set(const list& v) {
    this->line_17_target = jsCreateListCopy(v);
    this->line_17_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_17__time = this->line_17_time;
        this->line_17_time_set(0);

        if ((bool)(this->line_17__time)) {
            this->line_17_startRamp(v[0], this->line_17__time);
        } else {
            this->line_17_output_set(v[0]);
            this->line_17_startValue = v[0];
            this->line_17_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_17_time_set(0);
        this->line_17__time = (v[1] < 0 ? 0 : v[1]);
        this->line_17_startRamp(v[0], this->line_17__time);
    } else if (v->length == 3) {
        this->line_17_time_set(0);
        this->line_17_grain_set(v[2]);
        this->line_17__time = (v[1] < 0 ? 0 : v[1]);
        this->line_17_startRamp(v[0], this->line_17__time);
    } else {
        this->line_17_time_set(0);
        this->line_17_pendingRamps = jsCreateListCopy(v);
        this->line_17_startPendingRamp();
    }
}

void switchobj_02_out_number_set(number v) {
    {
        list converted = {v};
        this->line_17_target_set(converted);
    }
}

void switchobj_02_in2_number_set(number v) {
    if (this->switchobj_02_onoff == 2) {
        this->switchobj_02_out_number_set(v);
    }
}

void expr_09_out1_set(number v) {
    this->expr_09_out1 = v;
    this->numberobj_23_value_set(this->expr_09_out1);
    this->switchobj_02_in2_number_set(this->expr_09_out1);
}

void expr_09_in1_set(number in1) {
    this->expr_09_in1 = in1;
    this->expr_09_out1_set(this->expr_09_in1 + this->expr_09_in2);//#map:+_obj-146:1
}

void random_12_result_set(number v) {
    this->expr_09_in1_set(v);
}

void random_12_trigger_bang() {
    this->random_12_result_set(rnbo_abs(this->random_12_nz_next()) * this->random_12_limit);
}

void numberobj_20_output_set(number ) {}

void numberobj_20_value_set(number v) {
    this->numberobj_20_value_setter(v);
    v = this->numberobj_20_value;
    number localvalue = v;

    if (this->numberobj_20_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-142"), localvalue, this->_currentTime);
    this->numberobj_20_output_set(localvalue);
}

void switchobj_02_in1_number_set(number v) {
    if (this->switchobj_02_onoff == 1) {
        this->switchobj_02_out_number_set(v);
    }
}

void expr_08_out1_set(number v) {
    this->expr_08_out1 = v;
    this->numberobj_20_value_set(this->expr_08_out1);
    this->switchobj_02_in1_number_set(this->expr_08_out1);
}

void expr_08_in1_set(number in1) {
    this->expr_08_in1 = in1;
    this->expr_08_out1_set(this->expr_08_in1 + this->expr_08_in2);//#map:+_obj-148:1
}

void random_11_result_set(number v) {
    this->expr_08_in1_set(v);
}

void random_11_trigger_bang() {
    this->random_11_result_set(rnbo_abs(this->random_11_nz_next()) * this->random_11_limit);
}

void button_21_output_bang() {
    this->counter_05_input_bang();
    this->message_22_trigger_bang();
    this->random_12_trigger_bang();
    this->random_11_trigger_bang();
}

void button_21_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-143"), this->_currentTime);;
    this->button_21_output_bang();
}

void button_20_output_bang() {
    this->button_21_bangval_bang();
    this->random_10_trigger_bang();
}

void button_20_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-134"), this->_currentTime);;
    this->button_20_output_bang();
}

void counter_04_carry_set(number v) {
    this->counter_04_carry = v;
    this->button_20_bangval_bang();
}

void counter_04_underflow_bang_bang() {}

number counter_04_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_04_underflow_number_set(number v) {
    v = this->counter_04_underflow_number_constrain(v);
    this->counter_04_underflow_number = v;
}

void counter_04_input_bang() {
    this->counter_04_output_set(this->counter_04_count);
    this->counter_04_inc();

    if (this->counter_04_count > this->counter_04_maximum) {
        if (this->counter_04_direction == 2) {
            this->counter_04_phase = !(bool)(this->counter_04_phase);
            this->counter_04_count = this->counter_04_maximum;
            this->counter_04_inc();
        } else
            this->counter_04_count = this->counter_04_minimum;

        if ((bool)(!(bool)(this->counter_04_overflow_number))) {
            if (this->counter_04_carryflag == 1)
                this->counter_04_overflow_bang_bang();
            else
                this->counter_04_overflow_number_set(1);
        }

        this->counter_04_carry_set(this->counter_04_carry + 1);
    } else if ((bool)(this->counter_04_overflow_number) && this->counter_04_carryflag == 0)
        this->counter_04_overflow_number_set(0);

    if (this->counter_04_count < this->counter_04_minimum) {
        if (this->counter_04_direction == 2) {
            this->counter_04_phase = !(bool)(this->counter_04_phase);
            this->counter_04_count = this->counter_04_minimum;
            this->counter_04_inc();
        } else
            this->counter_04_count = this->counter_04_maximum;

        if ((bool)(!(bool)(this->counter_04_underflow_number))) {
            if (this->counter_04_carryflag == 1)
                this->counter_04_underflow_bang_bang();
            else
                this->counter_04_underflow_number_set(1);
        }

        this->counter_04_carry_set(this->counter_04_carry + 1);
    } else if ((bool)(this->counter_04_underflow_number) && this->counter_04_carryflag == 0)
        this->counter_04_underflow_number_set(0);
}

void metro_03_tickout_bang() {
    this->button_19_bangval_bang();
    this->counter_04_input_bang();
}

void message_22_set_set(const list& v) {
    this->message_22_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-141"), v, this->_currentTime);
}

void numberobj_25_output_set(number ) {}

void numberobj_25_value_set(number v) {
    this->numberobj_25_value_setter(v);
    v = this->numberobj_25_value;
    number localvalue = v;

    if (this->numberobj_25_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-175"), localvalue, this->_currentTime);
    this->numberobj_25_output_set(localvalue);
}

void button_22_output_bang() {}

void button_22_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-177"), this->_currentTime);;
    this->button_22_output_bang();
}

void counter_06_output_set(number v) {
    this->counter_06_output = v;
    this->numberobj_25_value_set(v);
}

void counter_06_overflow_bang_bang() {}

number counter_06_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_06_overflow_number_set(number v) {
    v = this->counter_06_overflow_number_constrain(v);
    this->counter_06_overflow_number = v;
}

void numberobj_30_output_set(number ) {}

void numberobj_30_value_set(number v) {
    this->numberobj_30_value_setter(v);
    v = this->numberobj_30_value;
    number localvalue = v;

    if (this->numberobj_30_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-180"), localvalue, this->_currentTime);
    this->numberobj_30_output_set(localvalue);
}

void switchobj_03_onoff_set(number v) {
    this->switchobj_03_onoff = v;
}

void expr_13_out1_set(number v) {
    this->expr_13_out1 = v;
    this->numberobj_30_value_set(this->expr_13_out1);
    this->switchobj_03_onoff_set(this->expr_13_out1);
}

void expr_13_in1_set(number in1) {
    this->expr_13_in1 = in1;
    this->expr_13_out1_set(this->expr_13_in1 + this->expr_13_in2);//#map:+_obj-162:1
}

void counter_07_output_set(number v) {
    this->counter_07_output = v;
    this->expr_13_in1_set(v);
}

void counter_07_overflow_bang_bang() {}

number counter_07_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_07_overflow_number_set(number v) {
    v = this->counter_07_overflow_number_constrain(v);
    this->counter_07_overflow_number = v;
}

void counter_07_carry_set(number v) {
    this->counter_07_carry = v;
}

void counter_07_underflow_bang_bang() {}

number counter_07_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_07_underflow_number_set(number v) {
    v = this->counter_07_underflow_number_constrain(v);
    this->counter_07_underflow_number = v;
}

void counter_07_input_bang() {
    this->counter_07_output_set(this->counter_07_count);
    this->counter_07_inc();

    if (this->counter_07_count > this->counter_07_maximum) {
        if (this->counter_07_direction == 2) {
            this->counter_07_phase = !(bool)(this->counter_07_phase);
            this->counter_07_count = this->counter_07_maximum;
            this->counter_07_inc();
        } else
            this->counter_07_count = this->counter_07_minimum;

        if ((bool)(!(bool)(this->counter_07_overflow_number))) {
            if (this->counter_07_carryflag == 1)
                this->counter_07_overflow_bang_bang();
            else
                this->counter_07_overflow_number_set(1);
        }

        this->counter_07_carry_set(this->counter_07_carry + 1);
    } else if ((bool)(this->counter_07_overflow_number) && this->counter_07_carryflag == 0)
        this->counter_07_overflow_number_set(0);

    if (this->counter_07_count < this->counter_07_minimum) {
        if (this->counter_07_direction == 2) {
            this->counter_07_phase = !(bool)(this->counter_07_phase);
            this->counter_07_count = this->counter_07_minimum;
            this->counter_07_inc();
        } else
            this->counter_07_count = this->counter_07_maximum;

        if ((bool)(!(bool)(this->counter_07_underflow_number))) {
            if (this->counter_07_carryflag == 1)
                this->counter_07_underflow_bang_bang();
            else
                this->counter_07_underflow_number_set(1);
        }

        this->counter_07_carry_set(this->counter_07_carry + 1);
    } else if ((bool)(this->counter_07_underflow_number) && this->counter_07_carryflag == 0)
        this->counter_07_underflow_number_set(0);
}

number line_18_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_18_time_set(number v) {
    v = this->line_18_time_constrain(v);
    this->line_18_time = v;
}

void message_23_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_18_time_set(converted);
    }
}

void message_23_trigger_bang() {
    this->message_23_out_set(this->message_23_set);
}

void numberobj_29_output_set(number ) {}

void numberobj_29_value_set(number v) {
    this->numberobj_29_value_setter(v);
    v = this->numberobj_29_value;
    number localvalue = v;

    if (this->numberobj_29_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-161"), localvalue, this->_currentTime);
    this->numberobj_29_output_set(localvalue);
}

void numberobj_28_output_set(number ) {}

void numberobj_28_value_set(number v) {
    this->numberobj_28_value_setter(v);
    v = this->numberobj_28_value;
    number localvalue = v;

    if (this->numberobj_28_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-168"), localvalue, this->_currentTime);
    this->numberobj_28_output_set(localvalue);
}

void line_18_output_set(number v) {
    this->line_18_output = v;
    this->numberobj_28_value_set(v);
    this->slider_06_input_set(v);
}

void line_18_stop_bang() {
    this->getEngine()->flushClockEvents(this, -470653941, false);;
    this->line_18_pendingRamps->length = 0;
    this->line_18_startValue = this->line_18_output;
    this->line_18_slope = 0;
    this->line_18__time = 0;
    this->line_18_time_set(0);
}

number line_18_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_18_grain_set(number v) {
    v = this->line_18_grain_constrain(v);
    this->line_18_grain = v;

    if ((bool)(!(bool)(this->line_18_isFinished(this->line_18_startValue)))) {
        this->getEngine()->flushClockEvents(this, -470653941, false);;
        this->line_18_scheduleNext();
    }
}

void line_18_end_bang() {}

void line_18_target_set(const list& v) {
    this->line_18_target = jsCreateListCopy(v);
    this->line_18_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_18__time = this->line_18_time;
        this->line_18_time_set(0);

        if ((bool)(this->line_18__time)) {
            this->line_18_startRamp(v[0], this->line_18__time);
        } else {
            this->line_18_output_set(v[0]);
            this->line_18_startValue = v[0];
            this->line_18_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_18_time_set(0);
        this->line_18__time = (v[1] < 0 ? 0 : v[1]);
        this->line_18_startRamp(v[0], this->line_18__time);
    } else if (v->length == 3) {
        this->line_18_time_set(0);
        this->line_18_grain_set(v[2]);
        this->line_18__time = (v[1] < 0 ? 0 : v[1]);
        this->line_18_startRamp(v[0], this->line_18__time);
    } else {
        this->line_18_time_set(0);
        this->line_18_pendingRamps = jsCreateListCopy(v);
        this->line_18_startPendingRamp();
    }
}

void switchobj_03_out_number_set(number v) {
    {
        list converted = {v};
        this->line_18_target_set(converted);
    }
}

void switchobj_03_in2_number_set(number v) {
    if (this->switchobj_03_onoff == 2) {
        this->switchobj_03_out_number_set(v);
    }
}

void expr_12_out1_set(number v) {
    this->expr_12_out1 = v;
    this->numberobj_29_value_set(this->expr_12_out1);
    this->switchobj_03_in2_number_set(this->expr_12_out1);
}

void expr_12_in1_set(number in1) {
    this->expr_12_in1 = in1;
    this->expr_12_out1_set(this->expr_12_in1 + this->expr_12_in2);//#map:+_obj-170:1
}

void random_15_result_set(number v) {
    this->expr_12_in1_set(v);
}

void random_15_trigger_bang() {
    this->random_15_result_set(rnbo_abs(this->random_15_nz_next()) * this->random_15_limit);
}

void numberobj_26_output_set(number ) {}

void numberobj_26_value_set(number v) {
    this->numberobj_26_value_setter(v);
    v = this->numberobj_26_value;
    number localvalue = v;

    if (this->numberobj_26_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-166"), localvalue, this->_currentTime);
    this->numberobj_26_output_set(localvalue);
}

void switchobj_03_in1_number_set(number v) {
    if (this->switchobj_03_onoff == 1) {
        this->switchobj_03_out_number_set(v);
    }
}

void expr_11_out1_set(number v) {
    this->expr_11_out1 = v;
    this->numberobj_26_value_set(this->expr_11_out1);
    this->switchobj_03_in1_number_set(this->expr_11_out1);
}

void expr_11_in1_set(number in1) {
    this->expr_11_in1 = in1;
    this->expr_11_out1_set(this->expr_11_in1 + this->expr_11_in2);//#map:+_obj-172:1
}

void random_14_result_set(number v) {
    this->expr_11_in1_set(v);
}

void random_14_trigger_bang() {
    this->random_14_result_set(rnbo_abs(this->random_14_nz_next()) * this->random_14_limit);
}

void button_24_output_bang() {
    this->counter_07_input_bang();
    this->message_23_trigger_bang();
    this->random_15_trigger_bang();
    this->random_14_trigger_bang();
}

void button_24_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-167"), this->_currentTime);;
    this->button_24_output_bang();
}

void button_23_output_bang() {
    this->button_24_bangval_bang();
    this->random_13_trigger_bang();
}

void button_23_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-158"), this->_currentTime);;
    this->button_23_output_bang();
}

void counter_06_carry_set(number v) {
    this->counter_06_carry = v;
    this->button_23_bangval_bang();
}

void counter_06_underflow_bang_bang() {}

number counter_06_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_06_underflow_number_set(number v) {
    v = this->counter_06_underflow_number_constrain(v);
    this->counter_06_underflow_number = v;
}

void counter_06_input_bang() {
    this->counter_06_output_set(this->counter_06_count);
    this->counter_06_inc();

    if (this->counter_06_count > this->counter_06_maximum) {
        if (this->counter_06_direction == 2) {
            this->counter_06_phase = !(bool)(this->counter_06_phase);
            this->counter_06_count = this->counter_06_maximum;
            this->counter_06_inc();
        } else
            this->counter_06_count = this->counter_06_minimum;

        if ((bool)(!(bool)(this->counter_06_overflow_number))) {
            if (this->counter_06_carryflag == 1)
                this->counter_06_overflow_bang_bang();
            else
                this->counter_06_overflow_number_set(1);
        }

        this->counter_06_carry_set(this->counter_06_carry + 1);
    } else if ((bool)(this->counter_06_overflow_number) && this->counter_06_carryflag == 0)
        this->counter_06_overflow_number_set(0);

    if (this->counter_06_count < this->counter_06_minimum) {
        if (this->counter_06_direction == 2) {
            this->counter_06_phase = !(bool)(this->counter_06_phase);
            this->counter_06_count = this->counter_06_minimum;
            this->counter_06_inc();
        } else
            this->counter_06_count = this->counter_06_maximum;

        if ((bool)(!(bool)(this->counter_06_underflow_number))) {
            if (this->counter_06_carryflag == 1)
                this->counter_06_underflow_bang_bang();
            else
                this->counter_06_underflow_number_set(1);
        }

        this->counter_06_carry_set(this->counter_06_carry + 1);
    } else if ((bool)(this->counter_06_underflow_number) && this->counter_06_carryflag == 0)
        this->counter_06_underflow_number_set(0);
}

void metro_04_tickout_bang() {
    this->button_22_bangval_bang();
    this->counter_06_input_bang();
}

void message_23_set_set(const list& v) {
    this->message_23_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-165"), v, this->_currentTime);
}

void numberobj_31_output_set(number ) {}

void numberobj_31_value_set(number v) {
    this->numberobj_31_value_setter(v);
    v = this->numberobj_31_value;
    number localvalue = v;

    if (this->numberobj_31_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-199"), localvalue, this->_currentTime);
    this->numberobj_31_output_set(localvalue);
}

void button_25_output_bang() {}

void button_25_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-201"), this->_currentTime);;
    this->button_25_output_bang();
}

void counter_08_output_set(number v) {
    this->counter_08_output = v;
    this->numberobj_31_value_set(v);
}

void counter_08_overflow_bang_bang() {}

number counter_08_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_08_overflow_number_set(number v) {
    v = this->counter_08_overflow_number_constrain(v);
    this->counter_08_overflow_number = v;
}

void numberobj_36_output_set(number ) {}

void numberobj_36_value_set(number v) {
    this->numberobj_36_value_setter(v);
    v = this->numberobj_36_value;
    number localvalue = v;

    if (this->numberobj_36_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-204"), localvalue, this->_currentTime);
    this->numberobj_36_output_set(localvalue);
}

void switchobj_04_onoff_set(number v) {
    this->switchobj_04_onoff = v;
}

void expr_16_out1_set(number v) {
    this->expr_16_out1 = v;
    this->numberobj_36_value_set(this->expr_16_out1);
    this->switchobj_04_onoff_set(this->expr_16_out1);
}

void expr_16_in1_set(number in1) {
    this->expr_16_in1 = in1;
    this->expr_16_out1_set(this->expr_16_in1 + this->expr_16_in2);//#map:+_obj-186:1
}

void counter_09_output_set(number v) {
    this->counter_09_output = v;
    this->expr_16_in1_set(v);
}

void counter_09_overflow_bang_bang() {}

number counter_09_overflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_09_overflow_number_set(number v) {
    v = this->counter_09_overflow_number_constrain(v);
    this->counter_09_overflow_number = v;
}

void counter_09_carry_set(number v) {
    this->counter_09_carry = v;
}

void counter_09_underflow_bang_bang() {}

number counter_09_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_09_underflow_number_set(number v) {
    v = this->counter_09_underflow_number_constrain(v);
    this->counter_09_underflow_number = v;
}

void counter_09_input_bang() {
    this->counter_09_output_set(this->counter_09_count);
    this->counter_09_inc();

    if (this->counter_09_count > this->counter_09_maximum) {
        if (this->counter_09_direction == 2) {
            this->counter_09_phase = !(bool)(this->counter_09_phase);
            this->counter_09_count = this->counter_09_maximum;
            this->counter_09_inc();
        } else
            this->counter_09_count = this->counter_09_minimum;

        if ((bool)(!(bool)(this->counter_09_overflow_number))) {
            if (this->counter_09_carryflag == 1)
                this->counter_09_overflow_bang_bang();
            else
                this->counter_09_overflow_number_set(1);
        }

        this->counter_09_carry_set(this->counter_09_carry + 1);
    } else if ((bool)(this->counter_09_overflow_number) && this->counter_09_carryflag == 0)
        this->counter_09_overflow_number_set(0);

    if (this->counter_09_count < this->counter_09_minimum) {
        if (this->counter_09_direction == 2) {
            this->counter_09_phase = !(bool)(this->counter_09_phase);
            this->counter_09_count = this->counter_09_minimum;
            this->counter_09_inc();
        } else
            this->counter_09_count = this->counter_09_maximum;

        if ((bool)(!(bool)(this->counter_09_underflow_number))) {
            if (this->counter_09_carryflag == 1)
                this->counter_09_underflow_bang_bang();
            else
                this->counter_09_underflow_number_set(1);
        }

        this->counter_09_carry_set(this->counter_09_carry + 1);
    } else if ((bool)(this->counter_09_underflow_number) && this->counter_09_carryflag == 0)
        this->counter_09_underflow_number_set(0);
}

number line_19_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_19_time_set(number v) {
    v = this->line_19_time_constrain(v);
    this->line_19_time = v;
}

void message_24_out_set(const list& v) {
    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->line_19_time_set(converted);
    }
}

void message_24_trigger_bang() {
    this->message_24_out_set(this->message_24_set);
}

void numberobj_35_output_set(number ) {}

void numberobj_35_value_set(number v) {
    this->numberobj_35_value_setter(v);
    v = this->numberobj_35_value;
    number localvalue = v;

    if (this->numberobj_35_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-185"), localvalue, this->_currentTime);
    this->numberobj_35_output_set(localvalue);
}

void numberobj_34_output_set(number ) {}

void numberobj_34_value_set(number v) {
    this->numberobj_34_value_setter(v);
    v = this->numberobj_34_value;
    number localvalue = v;

    if (this->numberobj_34_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-192"), localvalue, this->_currentTime);
    this->numberobj_34_output_set(localvalue);
}

void line_19_output_set(number v) {
    this->line_19_output = v;
    this->numberobj_34_value_set(v);
    this->slider_08_input_set(v);
}

void line_19_stop_bang() {
    this->getEngine()->flushClockEvents(this, -842661172, false);;
    this->line_19_pendingRamps->length = 0;
    this->line_19_startValue = this->line_19_output;
    this->line_19_slope = 0;
    this->line_19__time = 0;
    this->line_19_time_set(0);
}

number line_19_grain_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void line_19_grain_set(number v) {
    v = this->line_19_grain_constrain(v);
    this->line_19_grain = v;

    if ((bool)(!(bool)(this->line_19_isFinished(this->line_19_startValue)))) {
        this->getEngine()->flushClockEvents(this, -842661172, false);;
        this->line_19_scheduleNext();
    }
}

void line_19_end_bang() {}

void line_19_target_set(const list& v) {
    this->line_19_target = jsCreateListCopy(v);
    this->line_19_pendingRamps->length = 0;

    if (v->length == 1) {
        this->line_19__time = this->line_19_time;
        this->line_19_time_set(0);

        if ((bool)(this->line_19__time)) {
            this->line_19_startRamp(v[0], this->line_19__time);
        } else {
            this->line_19_output_set(v[0]);
            this->line_19_startValue = v[0];
            this->line_19_stop_bang();
        }
    } else if (v->length == 2) {
        this->line_19_time_set(0);
        this->line_19__time = (v[1] < 0 ? 0 : v[1]);
        this->line_19_startRamp(v[0], this->line_19__time);
    } else if (v->length == 3) {
        this->line_19_time_set(0);
        this->line_19_grain_set(v[2]);
        this->line_19__time = (v[1] < 0 ? 0 : v[1]);
        this->line_19_startRamp(v[0], this->line_19__time);
    } else {
        this->line_19_time_set(0);
        this->line_19_pendingRamps = jsCreateListCopy(v);
        this->line_19_startPendingRamp();
    }
}

void switchobj_04_out_number_set(number v) {
    {
        list converted = {v};
        this->line_19_target_set(converted);
    }
}

void switchobj_04_in2_number_set(number v) {
    if (this->switchobj_04_onoff == 2) {
        this->switchobj_04_out_number_set(v);
    }
}

void expr_15_out1_set(number v) {
    this->expr_15_out1 = v;
    this->numberobj_35_value_set(this->expr_15_out1);
    this->switchobj_04_in2_number_set(this->expr_15_out1);
}

void expr_15_in1_set(number in1) {
    this->expr_15_in1 = in1;
    this->expr_15_out1_set(this->expr_15_in1 + this->expr_15_in2);//#map:+_obj-194:1
}

void random_18_result_set(number v) {
    this->expr_15_in1_set(v);
}

void random_18_trigger_bang() {
    this->random_18_result_set(rnbo_abs(this->random_18_nz_next()) * this->random_18_limit);
}

void numberobj_32_output_set(number ) {}

void numberobj_32_value_set(number v) {
    this->numberobj_32_value_setter(v);
    v = this->numberobj_32_value;
    number localvalue = v;

    if (this->numberobj_32_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->getEngine()->sendNumMessage(TAG("valout"), TAG("number_obj-190"), localvalue, this->_currentTime);
    this->numberobj_32_output_set(localvalue);
}

void switchobj_04_in1_number_set(number v) {
    if (this->switchobj_04_onoff == 1) {
        this->switchobj_04_out_number_set(v);
    }
}

void expr_14_out1_set(number v) {
    this->expr_14_out1 = v;
    this->numberobj_32_value_set(this->expr_14_out1);
    this->switchobj_04_in1_number_set(this->expr_14_out1);
}

void expr_14_in1_set(number in1) {
    this->expr_14_in1 = in1;
    this->expr_14_out1_set(this->expr_14_in1 + this->expr_14_in2);//#map:+_obj-196:1
}

void random_17_result_set(number v) {
    this->expr_14_in1_set(v);
}

void random_17_trigger_bang() {
    this->random_17_result_set(rnbo_abs(this->random_17_nz_next()) * this->random_17_limit);
}

void button_27_output_bang() {
    this->counter_09_input_bang();
    this->message_24_trigger_bang();
    this->random_18_trigger_bang();
    this->random_17_trigger_bang();
}

void button_27_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-191"), this->_currentTime);;
    this->button_27_output_bang();
}

void button_26_output_bang() {
    this->button_27_bangval_bang();
    this->random_16_trigger_bang();
}

void button_26_bangval_bang() {
    this->getEngine()->sendBangMessage(TAG("bangout"), TAG("button_obj-182"), this->_currentTime);;
    this->button_26_output_bang();
}

void counter_08_carry_set(number v) {
    this->counter_08_carry = v;
    this->button_26_bangval_bang();
}

void counter_08_underflow_bang_bang() {}

number counter_08_underflow_number_constrain(number v) const {
    if (v < 0)
        v = 0;

    if (v > 1)
        v = 1;

    return v;
}

void counter_08_underflow_number_set(number v) {
    v = this->counter_08_underflow_number_constrain(v);
    this->counter_08_underflow_number = v;
}

void counter_08_input_bang() {
    this->counter_08_output_set(this->counter_08_count);
    this->counter_08_inc();

    if (this->counter_08_count > this->counter_08_maximum) {
        if (this->counter_08_direction == 2) {
            this->counter_08_phase = !(bool)(this->counter_08_phase);
            this->counter_08_count = this->counter_08_maximum;
            this->counter_08_inc();
        } else
            this->counter_08_count = this->counter_08_minimum;

        if ((bool)(!(bool)(this->counter_08_overflow_number))) {
            if (this->counter_08_carryflag == 1)
                this->counter_08_overflow_bang_bang();
            else
                this->counter_08_overflow_number_set(1);
        }

        this->counter_08_carry_set(this->counter_08_carry + 1);
    } else if ((bool)(this->counter_08_overflow_number) && this->counter_08_carryflag == 0)
        this->counter_08_overflow_number_set(0);

    if (this->counter_08_count < this->counter_08_minimum) {
        if (this->counter_08_direction == 2) {
            this->counter_08_phase = !(bool)(this->counter_08_phase);
            this->counter_08_count = this->counter_08_minimum;
            this->counter_08_inc();
        } else
            this->counter_08_count = this->counter_08_maximum;

        if ((bool)(!(bool)(this->counter_08_underflow_number))) {
            if (this->counter_08_carryflag == 1)
                this->counter_08_underflow_bang_bang();
            else
                this->counter_08_underflow_number_set(1);
        }

        this->counter_08_carry_set(this->counter_08_carry + 1);
    } else if ((bool)(this->counter_08_underflow_number) && this->counter_08_carryflag == 0)
        this->counter_08_underflow_number_set(0);
}

void metro_05_tickout_bang() {
    this->button_25_bangval_bang();
    this->counter_08_input_bang();
}

void message_24_set_set(const list& v) {
    this->message_24_set = jsCreateListCopy(v);
    this->getEngine()->sendListMessage(TAG("listout"), TAG("message_obj-189"), v, this->_currentTime);
}

void metro_01_interval_set(number v) {
    this->metro_01_interval_setter(v);
    v = this->metro_01_interval;
}

void timevalue_01_out_set(number v) {
    this->metro_01_interval_set(v);
}

void metro_02_interval_set(number v) {
    this->metro_02_interval_setter(v);
    v = this->metro_02_interval;
}

void timevalue_02_out_set(number v) {
    this->metro_02_interval_set(v);
}

void metro_03_interval_set(number v) {
    this->metro_03_interval_setter(v);
    v = this->metro_03_interval;
}

void timevalue_03_out_set(number v) {
    this->metro_03_interval_set(v);
}

void metro_04_interval_set(number v) {
    this->metro_04_interval_setter(v);
    v = this->metro_04_interval;
}

void timevalue_04_out_set(number v) {
    this->metro_04_interval_set(v);
}

void metro_05_interval_set(number v) {
    this->metro_05_interval_setter(v);
    v = this->metro_05_interval;
}

void timevalue_05_out_set(number v) {
    this->metro_05_interval_set(v);
}

void transport_01_outtempo_set(number ) {}

void transport_01_state_set(number ) {}

void transport_01_outtimesig_set(const list& ) {}

void groove_01_perform(
    number rate_auto,
    number begin,
    number end,
    SampleValue * out1,
    SampleValue * sync,
    Index n
) {
    RNBO_UNUSED(out1);
    RNBO_UNUSED(end);
    RNBO_UNUSED(begin);
    auto __groove_01_crossfade = this->groove_01_crossfade;
    auto __groove_01_loop = this->groove_01_loop;
    auto __groove_01_playStatus = this->groove_01_playStatus;
    auto __groove_01_readIndex = this->groove_01_readIndex;
    auto __groove_01_incomingChange = this->groove_01_incomingChange;
    auto __groove_01_changeIncomingInSamples = this->groove_01_changeIncomingInSamples;
    auto __groove_01_buffer = this->groove_01_buffer;
    SampleArray<1> out = {out1};
    SampleIndex bufferLength = (SampleIndex)(__groove_01_buffer->getSize());
    Index i = 0;

    if (bufferLength > 1) {
        auto effectiveChannels = this->minimum(__groove_01_buffer->getChannels(), 1);
        number srMult = 0.001 * __groove_01_buffer->getSampleRate();
        number srInv = (number)1 / this->samplerate();
        number rateMult = __groove_01_buffer->getSampleRate() * srInv;

        for (; i < n; i++) {
            Index channel = 0;
            number offset = 0;
            number loopMin = 0 * srMult;
            loopMin = (loopMin > bufferLength - 1 ? bufferLength - 1 : (loopMin < 0 ? 0 : loopMin));
            number loopMax = bufferLength;
            loopMax = (loopMax > bufferLength ? bufferLength : (loopMax < 0 ? 0 : loopMax));

            if (loopMin >= loopMax) {
                offset = loopMax;
                loopMax = bufferLength;
                loopMin -= offset;
            }

            number loopLength = loopMax - loopMin;
            number currentRate = rate_auto * rateMult;
            number currentSync = 0;

            if (__groove_01_changeIncomingInSamples > 0) {
                __groove_01_changeIncomingInSamples--;

                if (__groove_01_changeIncomingInSamples <= 0) {
                    if (__groove_01_incomingChange == 1) {
                        if (currentRate < 0) {
                            __groove_01_readIndex = loopMax - 1;
                        } else {
                            __groove_01_readIndex = loopMin;
                        }

                        __groove_01_playStatus = 1;
                    } else if (__groove_01_incomingChange == 0) {
                        __groove_01_playStatus = 0;
                    }

                    __groove_01_incomingChange = 2;
                }
            }

            if (loopLength > 0) {
                if (currentRate != 0) {
                    if (__groove_01_playStatus == 1) {
                        if ((bool)(__groove_01_loop)) {
                            while (__groove_01_readIndex < loopMin) {
                                __groove_01_readIndex += loopLength;
                            }

                            while (__groove_01_readIndex >= loopMax) {
                                __groove_01_readIndex -= loopLength;
                            }
                        } else if (__groove_01_readIndex >= loopMax || __groove_01_readIndex < loopMin) {
                            __groove_01_playStatus = 0;
                            break;
                        }

                        for (; channel < effectiveChannels; channel++) {
                            number outSample = (currentRate == 1 ? this->groove_01_getSample((Index)(channel), trunc(__groove_01_readIndex), offset, bufferLength) : this->groove_01_interpolatedSample(
                                (Index)(channel),
                                __groove_01_readIndex,
                                loopMax,
                                loopLength,
                                offset,
                                bufferLength
                            ));

                            if (__groove_01_crossfade > 0) {
                                out[(Index)channel][(Index)i] = this->groove_01_crossfadedSample(
                                    outSample,
                                    __groove_01_readIndex,
                                    (Index)(channel),
                                    currentRate,
                                    loopMin,
                                    loopMax,
                                    loopLength,
                                    offset,
                                    bufferLength
                                );
                            } else {
                                out[(Index)channel][(Index)i] = outSample;
                            }
                        }

                        __groove_01_readIndex += currentRate;
                    }
                }
            }

            for (; channel < 1; channel++) {
                if (__groove_01_playStatus <= 0)
                    sync[(Index)i] = 0;

                out[(Index)channel][(Index)i] = 0;
            }
        }
    }

    for (; i < n; i++) {
        if (__groove_01_playStatus <= 0)
            sync[(Index)i] = 0;

        for (number channel = 0; channel < 1; channel++) {
            out[(Index)channel][(Index)i] = 0;
        }
    }

    this->groove_01_changeIncomingInSamples = __groove_01_changeIncomingInSamples;
    this->groove_01_incomingChange = __groove_01_incomingChange;
    this->groove_01_readIndex = __groove_01_readIndex;
    this->groove_01_playStatus = __groove_01_playStatus;
}

void gaintilde_02_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_02_interp = this->gaintilde_02_interp;
    auto __gaintilde_02_loginc = this->gaintilde_02_loginc;
    auto __gaintilde_02_zval = this->gaintilde_02_zval;
    auto __gaintilde_02_value = this->gaintilde_02_value;
    number mult = (__gaintilde_02_value <= 0 ? 0. : __gaintilde_02_zval * rnbo_exp(__gaintilde_02_value * __gaintilde_02_loginc));
    auto iv = this->mstosamps(__gaintilde_02_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_02_ramp_next(mult, iv, iv);
    }
}

void gaintilde_01_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_01_interp = this->gaintilde_01_interp;
    auto __gaintilde_01_loginc = this->gaintilde_01_loginc;
    auto __gaintilde_01_zval = this->gaintilde_01_zval;
    auto __gaintilde_01_value = this->gaintilde_01_value;
    number mult = (__gaintilde_01_value <= 0 ? 0. : __gaintilde_01_zval * rnbo_exp(__gaintilde_01_value * __gaintilde_01_loginc));
    auto iv = this->mstosamps(__gaintilde_01_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_01_ramp_next(mult, iv, iv);
    }
}

void groove_02_perform(
    number rate_auto,
    number begin,
    number end,
    SampleValue * out1,
    SampleValue * sync,
    Index n
) {
    RNBO_UNUSED(out1);
    RNBO_UNUSED(end);
    RNBO_UNUSED(begin);
    auto __groove_02_crossfade = this->groove_02_crossfade;
    auto __groove_02_loop = this->groove_02_loop;
    auto __groove_02_playStatus = this->groove_02_playStatus;
    auto __groove_02_readIndex = this->groove_02_readIndex;
    auto __groove_02_incomingChange = this->groove_02_incomingChange;
    auto __groove_02_changeIncomingInSamples = this->groove_02_changeIncomingInSamples;
    auto __groove_02_buffer = this->groove_02_buffer;
    SampleArray<1> out = {out1};
    SampleIndex bufferLength = (SampleIndex)(__groove_02_buffer->getSize());
    Index i = 0;

    if (bufferLength > 1) {
        auto effectiveChannels = this->minimum(__groove_02_buffer->getChannels(), 1);
        number srMult = 0.001 * __groove_02_buffer->getSampleRate();
        number srInv = (number)1 / this->samplerate();
        number rateMult = __groove_02_buffer->getSampleRate() * srInv;

        for (; i < n; i++) {
            Index channel = 0;
            number offset = 0;
            number loopMin = 0 * srMult;
            loopMin = (loopMin > bufferLength - 1 ? bufferLength - 1 : (loopMin < 0 ? 0 : loopMin));
            number loopMax = bufferLength;
            loopMax = (loopMax > bufferLength ? bufferLength : (loopMax < 0 ? 0 : loopMax));

            if (loopMin >= loopMax) {
                offset = loopMax;
                loopMax = bufferLength;
                loopMin -= offset;
            }

            number loopLength = loopMax - loopMin;
            number currentRate = rate_auto * rateMult;
            number currentSync = 0;

            if (__groove_02_changeIncomingInSamples > 0) {
                __groove_02_changeIncomingInSamples--;

                if (__groove_02_changeIncomingInSamples <= 0) {
                    if (__groove_02_incomingChange == 1) {
                        if (currentRate < 0) {
                            __groove_02_readIndex = loopMax - 1;
                        } else {
                            __groove_02_readIndex = loopMin;
                        }

                        __groove_02_playStatus = 1;
                    } else if (__groove_02_incomingChange == 0) {
                        __groove_02_playStatus = 0;
                    }

                    __groove_02_incomingChange = 2;
                }
            }

            if (loopLength > 0) {
                if (currentRate != 0) {
                    if (__groove_02_playStatus == 1) {
                        if ((bool)(__groove_02_loop)) {
                            while (__groove_02_readIndex < loopMin) {
                                __groove_02_readIndex += loopLength;
                            }

                            while (__groove_02_readIndex >= loopMax) {
                                __groove_02_readIndex -= loopLength;
                            }
                        } else if (__groove_02_readIndex >= loopMax || __groove_02_readIndex < loopMin) {
                            __groove_02_playStatus = 0;
                            break;
                        }

                        for (; channel < effectiveChannels; channel++) {
                            number outSample = (currentRate == 1 ? this->groove_02_getSample((Index)(channel), trunc(__groove_02_readIndex), offset, bufferLength) : this->groove_02_interpolatedSample(
                                (Index)(channel),
                                __groove_02_readIndex,
                                loopMax,
                                loopLength,
                                offset,
                                bufferLength
                            ));

                            if (__groove_02_crossfade > 0) {
                                out[(Index)channel][(Index)i] = this->groove_02_crossfadedSample(
                                    outSample,
                                    __groove_02_readIndex,
                                    (Index)(channel),
                                    currentRate,
                                    loopMin,
                                    loopMax,
                                    loopLength,
                                    offset,
                                    bufferLength
                                );
                            } else {
                                out[(Index)channel][(Index)i] = outSample;
                            }
                        }

                        __groove_02_readIndex += currentRate;
                    }
                }
            }

            for (; channel < 1; channel++) {
                if (__groove_02_playStatus <= 0)
                    sync[(Index)i] = 0;

                out[(Index)channel][(Index)i] = 0;
            }
        }
    }

    for (; i < n; i++) {
        if (__groove_02_playStatus <= 0)
            sync[(Index)i] = 0;

        for (number channel = 0; channel < 1; channel++) {
            out[(Index)channel][(Index)i] = 0;
        }
    }

    this->groove_02_changeIncomingInSamples = __groove_02_changeIncomingInSamples;
    this->groove_02_incomingChange = __groove_02_incomingChange;
    this->groove_02_readIndex = __groove_02_readIndex;
    this->groove_02_playStatus = __groove_02_playStatus;
}

void gaintilde_03_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_03_interp = this->gaintilde_03_interp;
    auto __gaintilde_03_loginc = this->gaintilde_03_loginc;
    auto __gaintilde_03_zval = this->gaintilde_03_zval;
    auto __gaintilde_03_value = this->gaintilde_03_value;
    number mult = (__gaintilde_03_value <= 0 ? 0. : __gaintilde_03_zval * rnbo_exp(__gaintilde_03_value * __gaintilde_03_loginc));
    auto iv = this->mstosamps(__gaintilde_03_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_03_ramp_next(mult, iv, iv);
    }
}

void gaintilde_04_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_04_interp = this->gaintilde_04_interp;
    auto __gaintilde_04_loginc = this->gaintilde_04_loginc;
    auto __gaintilde_04_zval = this->gaintilde_04_zval;
    auto __gaintilde_04_value = this->gaintilde_04_value;
    number mult = (__gaintilde_04_value <= 0 ? 0. : __gaintilde_04_zval * rnbo_exp(__gaintilde_04_value * __gaintilde_04_loginc));
    auto iv = this->mstosamps(__gaintilde_04_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_04_ramp_next(mult, iv, iv);
    }
}

void groove_03_perform(
    number rate_auto,
    number begin,
    number end,
    SampleValue * out1,
    SampleValue * out2,
    SampleValue * sync,
    Index n
) {
    RNBO_UNUSED(out2);
    RNBO_UNUSED(out1);
    RNBO_UNUSED(end);
    RNBO_UNUSED(begin);
    auto __groove_03_crossfade = this->groove_03_crossfade;
    auto __groove_03_loop = this->groove_03_loop;
    auto __groove_03_playStatus = this->groove_03_playStatus;
    auto __groove_03_readIndex = this->groove_03_readIndex;
    auto __groove_03_incomingChange = this->groove_03_incomingChange;
    auto __groove_03_changeIncomingInSamples = this->groove_03_changeIncomingInSamples;
    auto __groove_03_buffer = this->groove_03_buffer;
    SampleArray<2> out = {out1, out2};
    SampleIndex bufferLength = (SampleIndex)(__groove_03_buffer->getSize());
    Index i = 0;

    if (bufferLength > 1) {
        auto effectiveChannels = this->minimum(__groove_03_buffer->getChannels(), 2);
        number srMult = 0.001 * __groove_03_buffer->getSampleRate();
        number srInv = (number)1 / this->samplerate();
        number rateMult = __groove_03_buffer->getSampleRate() * srInv;

        for (; i < n; i++) {
            Index channel = 0;
            number offset = 0;
            number loopMin = 0 * srMult;
            loopMin = (loopMin > bufferLength - 1 ? bufferLength - 1 : (loopMin < 0 ? 0 : loopMin));
            number loopMax = bufferLength;
            loopMax = (loopMax > bufferLength ? bufferLength : (loopMax < 0 ? 0 : loopMax));

            if (loopMin >= loopMax) {
                offset = loopMax;
                loopMax = bufferLength;
                loopMin -= offset;
            }

            number loopLength = loopMax - loopMin;
            number currentRate = rate_auto * rateMult;
            number currentSync = 0;

            if (__groove_03_changeIncomingInSamples > 0) {
                __groove_03_changeIncomingInSamples--;

                if (__groove_03_changeIncomingInSamples <= 0) {
                    if (__groove_03_incomingChange == 1) {
                        if (currentRate < 0) {
                            __groove_03_readIndex = loopMax - 1;
                        } else {
                            __groove_03_readIndex = loopMin;
                        }

                        __groove_03_playStatus = 1;
                    } else if (__groove_03_incomingChange == 0) {
                        __groove_03_playStatus = 0;
                    }

                    __groove_03_incomingChange = 2;
                }
            }

            if (loopLength > 0) {
                if (currentRate != 0) {
                    if (__groove_03_playStatus == 1) {
                        if ((bool)(__groove_03_loop)) {
                            while (__groove_03_readIndex < loopMin) {
                                __groove_03_readIndex += loopLength;
                            }

                            while (__groove_03_readIndex >= loopMax) {
                                __groove_03_readIndex -= loopLength;
                            }
                        } else if (__groove_03_readIndex >= loopMax || __groove_03_readIndex < loopMin) {
                            __groove_03_playStatus = 0;
                            break;
                        }

                        for (; channel < effectiveChannels; channel++) {
                            number outSample = (currentRate == 1 ? this->groove_03_getSample((Index)(channel), trunc(__groove_03_readIndex), offset, bufferLength) : this->groove_03_interpolatedSample(
                                (Index)(channel),
                                __groove_03_readIndex,
                                loopMax,
                                loopLength,
                                offset,
                                bufferLength
                            ));

                            if (__groove_03_crossfade > 0) {
                                out[(Index)channel][(Index)i] = this->groove_03_crossfadedSample(
                                    outSample,
                                    __groove_03_readIndex,
                                    (Index)(channel),
                                    currentRate,
                                    loopMin,
                                    loopMax,
                                    loopLength,
                                    offset,
                                    bufferLength
                                );
                            } else {
                                out[(Index)channel][(Index)i] = outSample;
                            }
                        }

                        __groove_03_readIndex += currentRate;
                    }
                }
            }

            for (; channel < 2; channel++) {
                if (__groove_03_playStatus <= 0)
                    sync[(Index)i] = 0;

                out[(Index)channel][(Index)i] = 0;
            }
        }
    }

    for (; i < n; i++) {
        if (__groove_03_playStatus <= 0)
            sync[(Index)i] = 0;

        for (number channel = 0; channel < 2; channel++) {
            out[(Index)channel][(Index)i] = 0;
        }
    }

    this->groove_03_changeIncomingInSamples = __groove_03_changeIncomingInSamples;
    this->groove_03_incomingChange = __groove_03_incomingChange;
    this->groove_03_readIndex = __groove_03_readIndex;
    this->groove_03_playStatus = __groove_03_playStatus;
}

void pan_tilde_02_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_02_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_02_func_next(pos, 0) * in0[(Index)i];
    }
}

void dspexpr_01_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * 1.2;//#map:_###_obj_###_:1
    }
}

void dspexpr_03_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * 1.2;//#map:_###_obj_###_:1
    }
}

void adsr_01_perform(
    number attack,
    number decay,
    number sustain,
    number release,
    const SampleValue * trigger_signal,
    SampleValue * out,
    Index n
) {
    RNBO_UNUSED(trigger_signal);
    RNBO_UNUSED(release);
    RNBO_UNUSED(sustain);
    RNBO_UNUSED(decay);
    RNBO_UNUSED(attack);
    auto __adsr_01_trigger_number = this->adsr_01_trigger_number;
    auto __adsr_01_time = this->adsr_01_time;
    auto __adsr_01_amplitude = this->adsr_01_amplitude;
    auto __adsr_01_outval = this->adsr_01_outval;
    auto __adsr_01_startingpoint = this->adsr_01_startingpoint;
    auto __adsr_01_phase = this->adsr_01_phase;
    auto __adsr_01_legato = this->adsr_01_legato;
    auto __adsr_01_lastTriggerVal = this->adsr_01_lastTriggerVal;
    auto __adsr_01_maxsustain = this->adsr_01_maxsustain;
    auto __adsr_01_mspersamp = this->adsr_01_mspersamp;
    bool bangMute = false;

    for (Index i = 0; i < n; i++) {
        number clampedattack = (0 > __adsr_01_mspersamp ? 0 : __adsr_01_mspersamp);
        number clampeddecay = (0 > __adsr_01_mspersamp ? 0 : __adsr_01_mspersamp);
        number clampedsustain = (__adsr_01_maxsustain > __adsr_01_mspersamp ? __adsr_01_maxsustain : __adsr_01_mspersamp);
        number clampedrelease = (0 > __adsr_01_mspersamp ? 0 : __adsr_01_mspersamp);
        number currentTriggerVal = this->adsr_01_triggerValueBuf[(Index)i];

        if ((__adsr_01_lastTriggerVal == 0.0 && currentTriggerVal != 0.0) || this->adsr_01_triggerBuf[(Index)i] == 1) {
            if ((bool)(__adsr_01_legato)) {
                if (__adsr_01_phase != 0) {
                    __adsr_01_startingpoint = __adsr_01_outval;
                } else {
                    __adsr_01_startingpoint = 0;
                }
            } else {
                __adsr_01_startingpoint = 0;
            }

            __adsr_01_amplitude = currentTriggerVal;
            __adsr_01_phase = 1;
            __adsr_01_time = 0.0;
            bangMute = false;
        } else if (__adsr_01_lastTriggerVal != 0.0 && currentTriggerVal == 0.0) {
            if (__adsr_01_phase != 4 && __adsr_01_phase != 0) {
                __adsr_01_phase = 4;
                __adsr_01_amplitude = __adsr_01_outval;
                __adsr_01_time = 0.0;
            }
        }

        __adsr_01_time += __adsr_01_mspersamp;

        if (__adsr_01_phase == 0) {
            __adsr_01_outval = 0;
        } else if (__adsr_01_phase == 1) {
            if (__adsr_01_time > clampedattack) {
                __adsr_01_time -= clampedattack;
                __adsr_01_phase = 2;
                __adsr_01_outval = __adsr_01_amplitude;
            } else {
                __adsr_01_outval = (__adsr_01_amplitude - __adsr_01_startingpoint) * __adsr_01_time / clampedattack + __adsr_01_startingpoint;
            }
        } else if (__adsr_01_phase == 2) {
            if (__adsr_01_time > clampeddecay) {
                __adsr_01_time -= clampeddecay;
                __adsr_01_phase = 3;
                __adsr_01_outval = __adsr_01_amplitude * 0;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * 0 + (__adsr_01_amplitude - __adsr_01_amplitude * 0) * (1. - __adsr_01_time / clampeddecay);
            }
        } else if (__adsr_01_phase == 3) {
            if (__adsr_01_time > clampedsustain && __adsr_01_maxsustain > -1) {
                __adsr_01_time -= clampedsustain;
                __adsr_01_phase = 4;
                __adsr_01_amplitude = __adsr_01_outval;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * 0;
            }
        } else if (__adsr_01_phase == 4) {
            if (__adsr_01_time > clampedrelease) {
                __adsr_01_time = 0;
                __adsr_01_phase = 0;
                __adsr_01_outval = 0;
                __adsr_01_amplitude = 0;
                bangMute = true;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * (1.0 - __adsr_01_time / clampedrelease);
            }
        }

        out[(Index)i] = __adsr_01_outval;
        this->adsr_01_triggerBuf[(Index)i] = 0;
        this->adsr_01_triggerValueBuf[(Index)i] = __adsr_01_trigger_number;
        __adsr_01_lastTriggerVal = currentTriggerVal;
    }

    if ((bool)(bangMute)) {
        this->getEngine()->scheduleClockEventWithValue(
            this,
            -1468824490,
            this->sampsToMs((SampleIndex)(this->vs)) + this->_currentTime,
            0
        );;
    }

    this->adsr_01_lastTriggerVal = __adsr_01_lastTriggerVal;
    this->adsr_01_phase = __adsr_01_phase;
    this->adsr_01_startingpoint = __adsr_01_startingpoint;
    this->adsr_01_outval = __adsr_01_outval;
    this->adsr_01_amplitude = __adsr_01_amplitude;
    this->adsr_01_time = __adsr_01_time;
}

void mtof_tilde_01_perform(number midivalue, SampleValue * out, Index n) {
    auto __mtof_tilde_01_base = this->mtof_tilde_01_base;

    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->mtof_tilde_01_innerMtoF_next(midivalue, __mtof_tilde_01_base);
    }
}

void cycle_tilde_01_perform(
    const Sample * frequency,
    number phase_offset,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(phase_offset);
    auto __cycle_tilde_01_f2i = this->cycle_tilde_01_f2i;
    auto __cycle_tilde_01_buffer = this->cycle_tilde_01_buffer;
    auto __cycle_tilde_01_phasei = this->cycle_tilde_01_phasei;
    Index i;

    for (i = 0; i < n; i++) {
        {
            uint32_t uint_phase;

            {
                {
                    uint_phase = __cycle_tilde_01_phasei;
                }
            }

            uint32_t idx = (uint32_t)(uint32_rshift(uint_phase, 18));
            number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
            number y0 = __cycle_tilde_01_buffer[(Index)idx];
            number y1 = __cycle_tilde_01_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
            number y = y0 + frac * (y1 - y0);

            {
                uint32_t pincr = (uint32_t)(uint32_trunc(frequency[(Index)i] * __cycle_tilde_01_f2i));
                __cycle_tilde_01_phasei = uint32_add(__cycle_tilde_01_phasei, pincr);
            }

            out1[(Index)i] = y;
            out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
            continue;
        }
    }

    this->cycle_tilde_01_phasei = __cycle_tilde_01_phasei;
}

void dspexpr_02_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void pan_tilde_01_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_01_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_01_func_next(pos, 0) * in0[(Index)i];
    }
}

void mtof_tilde_02_perform(number midivalue, SampleValue * out, Index n) {
    auto __mtof_tilde_02_base = this->mtof_tilde_02_base;

    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->mtof_tilde_02_innerMtoF_next(midivalue, __mtof_tilde_02_base);
    }
}

void rect_tilde_01_perform(
    const Sample * frequency,
    number pulsewidth,
    number syncPhase,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(syncPhase);
    RNBO_UNUSED(pulsewidth);
    auto __rect_tilde_01_xHistory = this->rect_tilde_01_xHistory;
    auto __rect_tilde_01_yHistory = this->rect_tilde_01_yHistory;
    auto __rect_tilde_01_didSync = this->rect_tilde_01_didSync;
    auto __rect_tilde_01_t = this->rect_tilde_01_t;
    auto __rect_tilde_01_lastSyncDiff = this->rect_tilde_01_lastSyncDiff;
    auto __rect_tilde_01_lastSyncPhase = this->rect_tilde_01_lastSyncPhase;
    Index i;

    for (i = 0; i < n; i++) {
        number __frequency = frequency[(Index)i];
        __frequency = rnbo_abs(__frequency);
        number dt = __frequency / this->samplerate();
        number pw = 0.5;
        number syncDiff = 0 - __rect_tilde_01_lastSyncPhase;
        __rect_tilde_01_lastSyncPhase = 0;
        __rect_tilde_01_lastSyncDiff = syncDiff;
        number syncLookahead = 0 + syncDiff;
        number tCurr = __rect_tilde_01_t;
        number tPrev = tCurr - dt;
        number tNext = tCurr + dt;

        if (tPrev < 0) {
            while (tPrev < 0) {
                tPrev += 1;
            }
        }

        if (tNext > 1) {
            while (tNext >= 1) {
                tNext -= 1;
            }
        }

        number yNext = this->rect_tilde_01_rectangle(tNext, pw);
        number yCurr = this->rect_tilde_01_rectangle(tCurr, pw);
        number yPrev = this->rect_tilde_01_rectangle(tPrev, pw);

        if (dt != 0.0) {
            if (yPrev < yCurr) {
                number d = tCurr / dt;
                yCurr += d - 0.5 * d * d - 0.5;
            } else if (yCurr < yNext) {
                number d = (1 - tCurr) / dt;
                yCurr += 0.5 * d * d + d + 0.5;
            } else if (yPrev > yCurr) {
                number d = (tCurr - pw) / dt;
                yCurr -= d - 0.5 * d * d - 0.5;
            } else if (yCurr > yNext) {
                number d = (pw - tCurr) / dt;
                yCurr -= 0.5 * d * d + d + 0.5;
            } else if ((bool)(__rect_tilde_01_didSync)) {
                yCurr = 0.25;
                __rect_tilde_01_didSync = false;
            } else if (syncLookahead > 1) {
                if (yCurr < 0) {
                    yCurr = -0.125;
                }

                __rect_tilde_01_t = 0;
                __rect_tilde_01_didSync = true;
            }

            __rect_tilde_01_t += dt;

            if (dt > 0) {
                while (__rect_tilde_01_t >= 1) {
                    __rect_tilde_01_t -= 1;
                }
            } else {
                while (__rect_tilde_01_t <= 0) {
                    __rect_tilde_01_t += 1;
                }
            }
        }

        number output = yCurr - __rect_tilde_01_yHistory + __rect_tilde_01_xHistory * 0.9997;
        __rect_tilde_01_xHistory = output;
        __rect_tilde_01_yHistory = yCurr;
        out1[(Index)i] = 0.5 * output;
        out2[(Index)i] = __rect_tilde_01_t;
    }

    this->rect_tilde_01_lastSyncPhase = __rect_tilde_01_lastSyncPhase;
    this->rect_tilde_01_lastSyncDiff = __rect_tilde_01_lastSyncDiff;
    this->rect_tilde_01_t = __rect_tilde_01_t;
    this->rect_tilde_01_didSync = __rect_tilde_01_didSync;
    this->rect_tilde_01_yHistory = __rect_tilde_01_yHistory;
    this->rect_tilde_01_xHistory = __rect_tilde_01_xHistory;
}

void dspexpr_04_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void pan_tilde_03_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_03_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_03_func_next(pos, 0) * in0[(Index)i];
    }
}

void dspexpr_05_perform(number in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    RNBO_UNUSED(in1);
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = 0 == 0;//#map:_###_obj_###_:1
    }
}

void mtof_tilde_03_perform(number midivalue, SampleValue * out, Index n) {
    auto __mtof_tilde_03_base = this->mtof_tilde_03_base;

    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->mtof_tilde_03_innerMtoF_next(midivalue, __mtof_tilde_03_base);
    }
}

void saw_tilde_01_perform(
    const Sample * frequency,
    number syncPhase,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(syncPhase);
    auto __saw_tilde_01_didSync = this->saw_tilde_01_didSync;
    auto __saw_tilde_01_lastSyncDiff = this->saw_tilde_01_lastSyncDiff;
    auto __saw_tilde_01_lastSyncPhase = this->saw_tilde_01_lastSyncPhase;
    auto __saw_tilde_01_t = this->saw_tilde_01_t;
    Index i;

    for (i = 0; i < n; i++) {
        number dt = frequency[(Index)i] / this->samplerate();
        number t1 = __saw_tilde_01_t + 0.5;
        t1 -= trunc(t1);
        number y = 2 * t1 - 1;

        if (dt != 0.0) {
            number syncDiff = 0 - __saw_tilde_01_lastSyncPhase;
            __saw_tilde_01_lastSyncPhase = 0;
            __saw_tilde_01_lastSyncDiff = syncDiff;
            number lookahead = 0 + syncDiff;

            if (t1 < dt) {
                number d = t1 / dt;
                y -= d + d - d * d - 1;
            } else if (t1 + dt > 1) {
                number d = (t1 - 1) / dt;
                y -= d + d + d * d + 1;
            } else if ((bool)(__saw_tilde_01_didSync)) {
                y = 0;
                __saw_tilde_01_didSync = false;
            } else if (lookahead > 1) {
                y *= 0.5;
                __saw_tilde_01_t = 0;
                __saw_tilde_01_didSync = true;
            }

            __saw_tilde_01_t += dt;

            if (dt > 0) {
                while (__saw_tilde_01_t >= 1) {
                    __saw_tilde_01_t -= 1;
                }
            } else {
                while (__saw_tilde_01_t <= 0) {
                    __saw_tilde_01_t += 1;
                }
            }
        }

        y = this->saw_tilde_01_dcblocker_next(y, 0.9997);
        out1[(Index)i] = 0.5 * y;
        out2[(Index)i] = __saw_tilde_01_t;
    }

    this->saw_tilde_01_t = __saw_tilde_01_t;
    this->saw_tilde_01_lastSyncPhase = __saw_tilde_01_lastSyncPhase;
    this->saw_tilde_01_lastSyncDiff = __saw_tilde_01_lastSyncDiff;
    this->saw_tilde_01_didSync = __saw_tilde_01_didSync;
}

void dspexpr_06_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void pan_tilde_04_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_04_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_04_func_next(pos, 0) * in0[(Index)i];
    }
}

void mtof_tilde_04_perform(number midivalue, SampleValue * out, Index n) {
    auto __mtof_tilde_04_base = this->mtof_tilde_04_base;

    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->mtof_tilde_04_innerMtoF_next(midivalue, __mtof_tilde_04_base);
    }
}

void phasor_01_perform(const Sample * freq, SampleValue * out, Index n) {
    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->phasor_01_ph_next(freq[(Index)i], -1);
        this->phasor_01_sigbuf[(Index)i] = -1;
    }
}

void triangle_tilde_01_perform(const Sample * phase, number duty, SampleValue * out1, Index n) {
    RNBO_UNUSED(duty);
    Index i;

    for (i = 0; i < n; i++) {
        number p1 = 0.5;
        auto wrappedPhase = this->wrap(phase[(Index)i], 0., 1.);
        p1 = (p1 > 1. ? 1. : (p1 < 0. ? 0. : p1));

        if (wrappedPhase < p1) {
            out1[(Index)i] = wrappedPhase / p1;
            continue;
        } else {
            out1[(Index)i] = (p1 == 1. ? wrappedPhase : 1. - (wrappedPhase - p1) / (1. - p1));
            continue;
        }
    }
}

void dspexpr_07_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void pan_tilde_06_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_06_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_06_func_next(pos, 0) * in0[(Index)i];
    }
}

void signaladder_01_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    const SampleValue * in4,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i] + in4[(Index)i];
    }
}

void gaintilde_07_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_07_interp = this->gaintilde_07_interp;
    auto __gaintilde_07_loginc = this->gaintilde_07_loginc;
    auto __gaintilde_07_zval = this->gaintilde_07_zval;
    auto __gaintilde_07_value = this->gaintilde_07_value;
    number mult = (__gaintilde_07_value <= 0 ? 0. : __gaintilde_07_zval * rnbo_exp(__gaintilde_07_value * __gaintilde_07_loginc));
    auto iv = this->mstosamps(__gaintilde_07_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_07_ramp_next(mult, iv, iv);
    }
}

void signaladder_02_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    const SampleValue * in4,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i] + in4[(Index)i];
    }
}

void gaintilde_08_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_08_interp = this->gaintilde_08_interp;
    auto __gaintilde_08_loginc = this->gaintilde_08_loginc;
    auto __gaintilde_08_zval = this->gaintilde_08_zval;
    auto __gaintilde_08_value = this->gaintilde_08_value;
    number mult = (__gaintilde_08_value <= 0 ? 0. : __gaintilde_08_zval * rnbo_exp(__gaintilde_08_value * __gaintilde_08_loginc));
    auto iv = this->mstosamps(__gaintilde_08_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_08_ramp_next(mult, iv, iv);
    }
}

void groove_04_perform(
    number rate_auto,
    number begin,
    number end,
    SampleValue * out1,
    SampleValue * sync,
    Index n
) {
    RNBO_UNUSED(out1);
    RNBO_UNUSED(end);
    RNBO_UNUSED(begin);
    RNBO_UNUSED(rate_auto);
    auto __groove_04_crossfade = this->groove_04_crossfade;
    auto __groove_04_loop = this->groove_04_loop;
    auto __groove_04_playStatus = this->groove_04_playStatus;
    auto __groove_04_readIndex = this->groove_04_readIndex;
    auto __groove_04_incomingChange = this->groove_04_incomingChange;
    auto __groove_04_changeIncomingInSamples = this->groove_04_changeIncomingInSamples;
    auto __groove_04_buffer = this->groove_04_buffer;
    SampleArray<1> out = {out1};
    SampleIndex bufferLength = (SampleIndex)(__groove_04_buffer->getSize());
    Index i = 0;

    if (bufferLength > 1) {
        auto effectiveChannels = this->minimum(__groove_04_buffer->getChannels(), 1);
        number srMult = 0.001 * __groove_04_buffer->getSampleRate();
        number srInv = (number)1 / this->samplerate();
        number rateMult = __groove_04_buffer->getSampleRate() * srInv;

        for (; i < n; i++) {
            Index channel = 0;
            number offset = 0;
            number loopMin = 0 * srMult;
            loopMin = (loopMin > bufferLength - 1 ? bufferLength - 1 : (loopMin < 0 ? 0 : loopMin));
            number loopMax = bufferLength;
            loopMax = (loopMax > bufferLength ? bufferLength : (loopMax < 0 ? 0 : loopMax));

            if (loopMin >= loopMax) {
                offset = loopMax;
                loopMax = bufferLength;
                loopMin -= offset;
            }

            number loopLength = loopMax - loopMin;
            number currentRate = 1 * rateMult;
            number currentSync = 0;

            if (__groove_04_changeIncomingInSamples > 0) {
                __groove_04_changeIncomingInSamples--;

                if (__groove_04_changeIncomingInSamples <= 0) {
                    if (__groove_04_incomingChange == 1) {
                        if (currentRate < 0) {
                            __groove_04_readIndex = loopMax - 1;
                        } else {
                            __groove_04_readIndex = loopMin;
                        }

                        __groove_04_playStatus = 1;
                    } else if (__groove_04_incomingChange == 0) {
                        __groove_04_playStatus = 0;
                    }

                    __groove_04_incomingChange = 2;
                }
            }

            if (loopLength > 0) {
                if (currentRate != 0) {
                    if (__groove_04_playStatus == 1) {
                        if ((bool)(__groove_04_loop)) {
                            while (__groove_04_readIndex < loopMin) {
                                __groove_04_readIndex += loopLength;
                            }

                            while (__groove_04_readIndex >= loopMax) {
                                __groove_04_readIndex -= loopLength;
                            }
                        } else if (__groove_04_readIndex >= loopMax || __groove_04_readIndex < loopMin) {
                            __groove_04_playStatus = 0;
                            break;
                        }

                        for (; channel < effectiveChannels; channel++) {
                            number outSample = (currentRate == 1 ? this->groove_04_getSample((Index)(channel), trunc(__groove_04_readIndex), offset, bufferLength) : this->groove_04_interpolatedSample(
                                (Index)(channel),
                                __groove_04_readIndex,
                                loopMax,
                                loopLength,
                                offset,
                                bufferLength
                            ));

                            if (__groove_04_crossfade > 0) {
                                out[(Index)channel][(Index)i] = this->groove_04_crossfadedSample(
                                    outSample,
                                    __groove_04_readIndex,
                                    (Index)(channel),
                                    currentRate,
                                    loopMin,
                                    loopMax,
                                    loopLength,
                                    offset,
                                    bufferLength
                                );
                            } else {
                                out[(Index)channel][(Index)i] = outSample;
                            }
                        }

                        __groove_04_readIndex += currentRate;
                    }
                }
            }

            for (; channel < 1; channel++) {
                if (__groove_04_playStatus <= 0)
                    sync[(Index)i] = 0;

                out[(Index)channel][(Index)i] = 0;
            }
        }
    }

    for (; i < n; i++) {
        if (__groove_04_playStatus <= 0)
            sync[(Index)i] = 0;

        for (number channel = 0; channel < 1; channel++) {
            out[(Index)channel][(Index)i] = 0;
        }
    }

    this->groove_04_changeIncomingInSamples = __groove_04_changeIncomingInSamples;
    this->groove_04_incomingChange = __groove_04_incomingChange;
    this->groove_04_readIndex = __groove_04_readIndex;
    this->groove_04_playStatus = __groove_04_playStatus;
}

void pan_tilde_05_perform(
    number pos,
    const SampleValue * in0,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out2[(Index)i] = this->pan_tilde_05_func_next(pos, 1) * in0[(Index)i];
        out1[(Index)i] = this->pan_tilde_05_func_next(pos, 0) * in0[(Index)i];
    }
}

void gaintilde_05_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_05_interp = this->gaintilde_05_interp;
    auto __gaintilde_05_loginc = this->gaintilde_05_loginc;
    auto __gaintilde_05_zval = this->gaintilde_05_zval;
    auto __gaintilde_05_value = this->gaintilde_05_value;
    number mult = (__gaintilde_05_value <= 0 ? 0. : __gaintilde_05_zval * rnbo_exp(__gaintilde_05_value * __gaintilde_05_loginc));
    auto iv = this->mstosamps(__gaintilde_05_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_05_ramp_next(mult, iv, iv);
    }
}

void signaladder_03_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    const SampleValue * in4,
    const SampleValue * in5,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i] + in4[(Index)i] + in5[(Index)i];
    }
}

void gaintilde_06_perform(const SampleValue * input_signal, SampleValue * output, Index n) {
    auto __gaintilde_06_interp = this->gaintilde_06_interp;
    auto __gaintilde_06_loginc = this->gaintilde_06_loginc;
    auto __gaintilde_06_zval = this->gaintilde_06_zval;
    auto __gaintilde_06_value = this->gaintilde_06_value;
    number mult = (__gaintilde_06_value <= 0 ? 0. : __gaintilde_06_zval * rnbo_exp(__gaintilde_06_value * __gaintilde_06_loginc));
    auto iv = this->mstosamps(__gaintilde_06_interp);

    for (Index i = 0; i < n; i++) {
        output[(Index)i] = input_signal[(Index)i] * this->gaintilde_06_ramp_next(mult, iv, iv);
    }
}

void signaladder_04_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    const SampleValue * in4,
    const SampleValue * in5,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i] + in4[(Index)i] + in5[(Index)i];
    }
}

void stackprotect_perform(Index n) {
    RNBO_UNUSED(n);
    auto __stackprotect_count = this->stackprotect_count;
    __stackprotect_count = 0;
    this->stackprotect_count = __stackprotect_count;
}

void data_01_srout_set(number ) {}

void data_01_chanout_set(number ) {}

void data_01_sizeout_set(number v) {
    this->data_01_sizeout = v;
}

void data_02_srout_set(number ) {}

void data_02_chanout_set(number ) {}

void data_02_sizeout_set(number v) {
    this->data_02_sizeout = v;
}

void data_03_srout_set(number ) {}

void data_03_chanout_set(number ) {}

void data_03_sizeout_set(number v) {
    this->data_03_sizeout = v;
}

void data_04_srout_set(number ) {}

void data_04_chanout_set(number ) {}

void data_04_sizeout_set(number v) {
    this->data_04_sizeout = v;
}

void data_05_srout_set(number ) {}

void data_05_chanout_set(number ) {}

void data_05_sizeout_set(number v) {
    this->data_05_sizeout = v;
}

void data_06_srout_set(number ) {}

void data_06_chanout_set(number ) {}

void data_06_sizeout_set(number v) {
    this->data_06_sizeout = v;
}

void data_07_srout_set(number ) {}

void data_07_chanout_set(number ) {}

void data_07_sizeout_set(number v) {
    this->data_07_sizeout = v;
}

void data_08_srout_set(number ) {}

void data_08_chanout_set(number ) {}

void data_08_sizeout_set(number v) {
    this->data_08_sizeout = v;
}

void toggle_01_value_number_setter(number v) {
    this->toggle_01_value_number = (v != 0 ? 1 : 0);
}

void toggle_02_value_number_setter(number v) {
    this->toggle_02_value_number = (v != 0 ? 1 : 0);
}

void random_01_limit_setter(number v) {
    if (v >= 0)
        this->random_01_limit = v;
}

void numberobj_01_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_01_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_01_value = localvalue;
}

void toggle_03_value_number_setter(number v) {
    this->toggle_03_value_number = (v != 0 ? 1 : 0);
}

void numberobj_02_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_02_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_02_value = localvalue;
}

void metro_01_interval_setter(number v) {
    this->metro_01_interval = (v > 0 ? v : 0);
}

void toggle_04_value_number_setter(number v) {
    this->toggle_04_value_number = (v != 0 ? 1 : 0);
}

void numberobj_03_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_03_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_03_value = localvalue;
}

void random_02_limit_setter(number v) {
    if (v >= 0)
        this->random_02_limit = v;
}

void numberobj_04_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_04_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_04_value = localvalue;
}

void numberobj_05_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_05_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_05_value = localvalue;
}

void random_03_limit_setter(number v) {
    if (v >= 0)
        this->random_03_limit = v;
}

void numberobj_06_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_06_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_06_value = localvalue;
}

void random_04_limit_setter(number v) {
    if (v >= 0)
        this->random_04_limit = v;
}

void numberobj_07_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_07_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_07_value = localvalue;
}

void random_05_limit_setter(number v) {
    if (v >= 0)
        this->random_05_limit = v;
}

void numberobj_08_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_08_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_08_value = localvalue;
}

void numberobj_09_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_09_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_09_value = localvalue;
}

void toggle_05_value_number_setter(number v) {
    this->toggle_05_value_number = (v != 0 ? 1 : 0);
}

void numberobj_10_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_10_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_10_value = localvalue;
}

void random_06_limit_setter(number v) {
    if (v >= 0)
        this->random_06_limit = v;
}

void numberobj_11_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_11_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_11_value = localvalue;
}

void toggle_06_value_number_setter(number v) {
    this->toggle_06_value_number = (v != 0 ? 1 : 0);
}

void metro_02_interval_setter(number v) {
    this->metro_02_interval = (v > 0 ? v : 0);
}

void random_07_limit_setter(number v) {
    if (v >= 0)
        this->random_07_limit = v;
}

void random_08_limit_setter(number v) {
    if (v >= 0)
        this->random_08_limit = v;
}

void numberobj_12_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_12_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_12_value = localvalue;
}

void numberobj_13_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_13_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_13_value = localvalue;
}

void numberobj_14_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_14_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_14_value = localvalue;
}

void numberobj_15_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_15_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_15_value = localvalue;
}

void random_09_limit_setter(number v) {
    if (v >= 0)
        this->random_09_limit = v;
}

void numberobj_16_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_16_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_16_value = localvalue;
}

void numberobj_17_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_17_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_17_value = localvalue;
}

void numberobj_18_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_18_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_18_value = localvalue;
}

void random_10_limit_setter(number v) {
    if (v >= 0)
        this->random_10_limit = v;
}

void numberobj_19_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_19_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_19_value = localvalue;
}

void toggle_07_value_number_setter(number v) {
    this->toggle_07_value_number = (v != 0 ? 1 : 0);
}

void metro_03_interval_setter(number v) {
    this->metro_03_interval = (v > 0 ? v : 0);
}

void random_11_limit_setter(number v) {
    if (v >= 0)
        this->random_11_limit = v;
}

void numberobj_20_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_20_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_20_value = localvalue;
}

void numberobj_21_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_21_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_21_value = localvalue;
}

void numberobj_22_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_22_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_22_value = localvalue;
}

void random_12_limit_setter(number v) {
    if (v >= 0)
        this->random_12_limit = v;
}

void numberobj_23_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_23_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_23_value = localvalue;
}

void numberobj_24_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_24_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_24_value = localvalue;
}

void random_13_limit_setter(number v) {
    if (v >= 0)
        this->random_13_limit = v;
}

void numberobj_25_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_25_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_25_value = localvalue;
}

void toggle_08_value_number_setter(number v) {
    this->toggle_08_value_number = (v != 0 ? 1 : 0);
}

void metro_04_interval_setter(number v) {
    this->metro_04_interval = (v > 0 ? v : 0);
}

void random_14_limit_setter(number v) {
    if (v >= 0)
        this->random_14_limit = v;
}

void numberobj_26_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_26_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_26_value = localvalue;
}

void numberobj_27_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_27_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_27_value = localvalue;
}

void numberobj_28_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_28_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_28_value = localvalue;
}

void random_15_limit_setter(number v) {
    if (v >= 0)
        this->random_15_limit = v;
}

void numberobj_29_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_29_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_29_value = localvalue;
}

void numberobj_30_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_30_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_30_value = localvalue;
}

void random_16_limit_setter(number v) {
    if (v >= 0)
        this->random_16_limit = v;
}

void numberobj_31_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_31_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_31_value = localvalue;
}

void toggle_09_value_number_setter(number v) {
    this->toggle_09_value_number = (v != 0 ? 1 : 0);
}

void metro_05_interval_setter(number v) {
    this->metro_05_interval = (v > 0 ? v : 0);
}

void random_17_limit_setter(number v) {
    if (v >= 0)
        this->random_17_limit = v;
}

void numberobj_32_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_32_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_32_value = localvalue;
}

void numberobj_33_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_33_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_33_value = localvalue;
}

void numberobj_34_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_34_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_34_value = localvalue;
}

void random_18_limit_setter(number v) {
    if (v >= 0)
        this->random_18_limit = v;
}

void numberobj_35_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_35_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_35_value = localvalue;
}

void numberobj_36_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_36_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_36_value = localvalue;
}

void toggle_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_01_value_number;
}

void toggle_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_01_value_number_set(preset["value"]);
}

void data_01_init() {
    this->data_01_buffer->setWantsFill(true);
}

Index data_01_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_01_dspsetup(bool force) {
    if ((bool)(this->data_01_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_01_sizemode == 2) {
        this->data_01_buffer = this->data_01_buffer->setSize((Index)(this->mstosamps(this->data_01_sizems)));
        updateDataRef(this, this->data_01_buffer);
    } else if (this->data_01_sizemode == 3) {
        this->data_01_buffer = this->data_01_buffer->setSize(this->data_01_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_01_buffer);
    }

    this->data_01_setupDone = true;
}

void data_01_bufferUpdated() {
    this->data_01_report();
}

void data_01_report() {
    this->data_01_srout_set(this->data_01_buffer->getSampleRate());
    this->data_01_chanout_set(this->data_01_buffer->getChannels());
    this->data_01_sizeout_set(this->data_01_buffer->getSize());
}

void data_02_init() {
    this->data_02_buffer->setWantsFill(true);
}

Index data_02_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_02_dspsetup(bool force) {
    if ((bool)(this->data_02_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_02_sizemode == 2) {
        this->data_02_buffer = this->data_02_buffer->setSize((Index)(this->mstosamps(this->data_02_sizems)));
        updateDataRef(this, this->data_02_buffer);
    } else if (this->data_02_sizemode == 3) {
        this->data_02_buffer = this->data_02_buffer->setSize(this->data_02_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_02_buffer);
    }

    this->data_02_setupDone = true;
}

void data_02_bufferUpdated() {
    this->data_02_report();
}

void data_02_report() {
    this->data_02_srout_set(this->data_02_buffer->getSampleRate());
    this->data_02_chanout_set(this->data_02_buffer->getChannels());
    this->data_02_sizeout_set(this->data_02_buffer->getSize());
}

void message_01_init() {
    this->message_01_set_set({130});
}

void data_03_init() {
    this->data_03_buffer->setWantsFill(true);
}

Index data_03_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_03_dspsetup(bool force) {
    if ((bool)(this->data_03_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_03_sizemode == 2) {
        this->data_03_buffer = this->data_03_buffer->setSize((Index)(this->mstosamps(this->data_03_sizems)));
        updateDataRef(this, this->data_03_buffer);
    } else if (this->data_03_sizemode == 3) {
        this->data_03_buffer = this->data_03_buffer->setSize(this->data_03_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_03_buffer);
    }

    this->data_03_setupDone = true;
}

void data_03_bufferUpdated() {
    this->data_03_report();
}

void data_03_report() {
    this->data_03_srout_set(this->data_03_buffer->getSampleRate());
    this->data_03_chanout_set(this->data_03_buffer->getChannels());
    this->data_03_sizeout_set(this->data_03_buffer->getSize());
}

void data_04_init() {
    this->data_04_buffer->setWantsFill(true);
}

Index data_04_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_04_dspsetup(bool force) {
    if ((bool)(this->data_04_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_04_sizemode == 2) {
        this->data_04_buffer = this->data_04_buffer->setSize((Index)(this->mstosamps(this->data_04_sizems)));
        updateDataRef(this, this->data_04_buffer);
    } else if (this->data_04_sizemode == 3) {
        this->data_04_buffer = this->data_04_buffer->setSize(this->data_04_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_04_buffer);
    }

    this->data_04_setupDone = true;
}

void data_04_bufferUpdated() {
    this->data_04_report();
}

void data_04_report() {
    this->data_04_srout_set(this->data_04_buffer->getSampleRate());
    this->data_04_chanout_set(this->data_04_buffer->getChannels());
    this->data_04_sizeout_set(this->data_04_buffer->getSize());
}

void data_05_init() {
    this->data_05_buffer->setWantsFill(true);
}

Index data_05_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_05_dspsetup(bool force) {
    if ((bool)(this->data_05_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_05_sizemode == 2) {
        this->data_05_buffer = this->data_05_buffer->setSize((Index)(this->mstosamps(this->data_05_sizems)));
        updateDataRef(this, this->data_05_buffer);
    } else if (this->data_05_sizemode == 3) {
        this->data_05_buffer = this->data_05_buffer->setSize(this->data_05_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_05_buffer);
    }

    this->data_05_setupDone = true;
}

void data_05_bufferUpdated() {
    this->data_05_report();
}

void data_05_report() {
    this->data_05_srout_set(this->data_05_buffer->getSampleRate());
    this->data_05_chanout_set(this->data_05_buffer->getChannels());
    this->data_05_sizeout_set(this->data_05_buffer->getSize());
}

void data_06_init() {
    this->data_06_buffer->setWantsFill(true);
}

Index data_06_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_06_dspsetup(bool force) {
    if ((bool)(this->data_06_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_06_sizemode == 2) {
        this->data_06_buffer = this->data_06_buffer->setSize((Index)(this->mstosamps(this->data_06_sizems)));
        updateDataRef(this, this->data_06_buffer);
    } else if (this->data_06_sizemode == 3) {
        this->data_06_buffer = this->data_06_buffer->setSize(this->data_06_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_06_buffer);
    }

    this->data_06_setupDone = true;
}

void data_06_bufferUpdated() {
    this->data_06_report();
}

void data_06_report() {
    this->data_06_srout_set(this->data_06_buffer->getSampleRate());
    this->data_06_chanout_set(this->data_06_buffer->getChannels());
    this->data_06_sizeout_set(this->data_06_buffer->getSize());
}

void param_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_01_value;
}

void param_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_01_value_set(preset["value"]);
}

number pan_tilde_01_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_01_func_reset() {}

void data_07_init() {
    this->data_07_buffer->setWantsFill(true);
}

Index data_07_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_07_dspsetup(bool force) {
    if ((bool)(this->data_07_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_07_sizemode == 2) {
        this->data_07_buffer = this->data_07_buffer->setSize((Index)(this->mstosamps(this->data_07_sizems)));
        updateDataRef(this, this->data_07_buffer);
    } else if (this->data_07_sizemode == 3) {
        this->data_07_buffer = this->data_07_buffer->setSize(this->data_07_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_07_buffer);
    }

    this->data_07_setupDone = true;
}

void data_07_bufferUpdated() {
    this->data_07_report();
}

void data_07_report() {
    this->data_07_srout_set(this->data_07_buffer->getSampleRate());
    this->data_07_chanout_set(this->data_07_buffer->getChannels());
    this->data_07_sizeout_set(this->data_07_buffer->getSize());
}

void data_08_init() {
    this->data_08_buffer->setWantsFill(true);
}

Index data_08_evaluateSizeExpr(number samplerate, number vectorsize) {
    RNBO_UNUSED(vectorsize);
    RNBO_UNUSED(samplerate);
    number size = 0;
    return (Index)(size);
}

void data_08_dspsetup(bool force) {
    if ((bool)(this->data_08_setupDone) && (bool)(!(bool)(force)))
        return;

    if (this->data_08_sizemode == 2) {
        this->data_08_buffer = this->data_08_buffer->setSize((Index)(this->mstosamps(this->data_08_sizems)));
        updateDataRef(this, this->data_08_buffer);
    } else if (this->data_08_sizemode == 3) {
        this->data_08_buffer = this->data_08_buffer->setSize(this->data_08_evaluateSizeExpr(this->samplerate(), this->vectorsize()));
        updateDataRef(this, this->data_08_buffer);
    }

    this->data_08_setupDone = true;
}

void data_08_bufferUpdated() {
    this->data_08_report();
}

void data_08_report() {
    this->data_08_srout_set(this->data_08_buffer->getSampleRate());
    this->data_08_chanout_set(this->data_08_buffer->getChannels());
    this->data_08_sizeout_set(this->data_08_buffer->getSize());
}

number groove_01_getSample(
    Index channel,
    SampleIndex index,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    if (offset > 0) {
        index += offset;

        if (index >= bufferLength)
            index -= bufferLength;
    }

    return this->groove_01_buffer->getSample(channel, index);
}

number groove_01_interpolatedSample(
    Index channel,
    number index,
    SampleIndex end,
    SampleIndex length,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    SampleIndex index1 = (SampleIndex)(trunc(index));
    number i_x = index - index1;
    number i_1px = 1. + i_x;
    number i_1mx = 1. - i_x;
    number i_2mx = 2. - i_x;
    number i_a = i_1mx * i_2mx;
    number i_b = i_1px * i_x;
    number i_p1 = -.1666667 * i_a * i_x;
    number i_p2 = .5 * i_1px * i_a;
    number i_p3 = .5 * i_b * i_2mx;
    number i_p4 = -.1666667 * i_b * i_1mx;
    SampleIndex index2 = (SampleIndex)(index1 + 1);

    if (index2 >= end)
        index2 -= length;

    SampleIndex index3 = (SampleIndex)(index1 + 2);

    if (index3 >= end)
        index3 -= length;

    SampleIndex index4 = (SampleIndex)(index1 + 3);

    if (index4 >= end)
        index4 -= length;

    return this->groove_01_getSample(channel, index1, offset, bufferLength) * i_p1 + this->groove_01_getSample(channel, index2, offset, bufferLength) * i_p2 + this->groove_01_getSample(channel, index3, offset, bufferLength) * i_p3 + this->groove_01_getSample(channel, index4, offset, bufferLength) * i_p4;
}

number groove_01_crossfadedSample(
    SampleValue out,
    number readIndex,
    Index channel,
    number rate,
    number loopMin,
    number loopMax,
    number loopLength,
    number offset,
    number bufferLength
) {
    auto crossFadeStart1 = this->maximum(loopMin - this->groove_01_crossfadeInSamples, 0);
    auto crossFadeEnd1 = this->minimum(crossFadeStart1 + this->groove_01_crossfadeInSamples, bufferLength);
    number crossFadeStart2 = crossFadeStart1 + loopLength;
    auto crossFadeEnd2 = this->minimum(crossFadeEnd1 + loopLength, bufferLength);
    number crossFadeLength = crossFadeEnd2 - crossFadeStart2;

    if (crossFadeLength > 0) {
        crossFadeEnd1 = crossFadeStart1 + crossFadeLength;
        number diff = -1;
        number addFactor = 0;

        if (readIndex >= crossFadeStart2) {
            diff = readIndex - crossFadeStart2;
            addFactor = -1;
        } else if (readIndex < crossFadeEnd1) {
            diff = crossFadeEnd1 - readIndex + loopMax - crossFadeStart2;
            addFactor = 1;
        }

        if (diff >= 0) {
            number out2ReadIndex = readIndex + loopLength * addFactor;
            number out2 = (rate == 1 ? this->groove_01_getSample(channel, trunc(out2ReadIndex), offset, bufferLength) : this->groove_01_interpolatedSample(channel, out2ReadIndex, loopMax, loopLength, offset, bufferLength));
            number out2Factor = diff / crossFadeLength;
            number out1Factor = 1 - out2Factor;
            return out * out1Factor + out2 * out2Factor;
        }
    }

    return out;
}

void groove_01_dspsetup(bool force) {
    if ((bool)(this->groove_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->groove_01_crossfadeInSamples = this->mstosamps(this->groove_01_crossfade);
    this->groove_01_setupDone = true;
}

number gaintilde_01_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_01_ramp_d_prev);
    this->gaintilde_01_ramp_d_prev = x;
    return temp;
}

void gaintilde_01_ramp_d_dspsetup() {
    this->gaintilde_01_ramp_d_reset();
}

void gaintilde_01_ramp_d_reset() {
    this->gaintilde_01_ramp_d_prev = 0;
}

number gaintilde_01_ramp_next(number x, number up, number down) {
    if (this->gaintilde_01_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_01_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_01_ramp_index = _up;
            this->gaintilde_01_ramp_increment = (x - this->gaintilde_01_ramp_prev) / _up;
        } else if (x < this->gaintilde_01_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_01_ramp_index = _down;
            this->gaintilde_01_ramp_increment = (x - this->gaintilde_01_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_01_ramp_index > 0) {
        this->gaintilde_01_ramp_prev += this->gaintilde_01_ramp_increment;
        this->gaintilde_01_ramp_index -= 1;
    } else {
        this->gaintilde_01_ramp_prev = x;
    }

    return this->gaintilde_01_ramp_prev;
}

void gaintilde_01_ramp_reset() {
    this->gaintilde_01_ramp_prev = 0;
    this->gaintilde_01_ramp_index = 0;
    this->gaintilde_01_ramp_increment = 0;
    this->gaintilde_01_ramp_d_reset();
}

void gaintilde_01_init() {
    this->gaintilde_01_loginc = rnbo_log(1.072);
    this->gaintilde_01_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_01_loginc));
}

void gaintilde_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_01_value;
}

void gaintilde_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_01_value_set(preset["value"]);
}

void gaintilde_01_dspsetup(bool force) {
    if ((bool)(this->gaintilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_01_setupDone = true;
    this->gaintilde_01_ramp_d_dspsetup();
}

number groove_02_getSample(
    Index channel,
    SampleIndex index,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    if (offset > 0) {
        index += offset;

        if (index >= bufferLength)
            index -= bufferLength;
    }

    return this->groove_02_buffer->getSample(channel, index);
}

number groove_02_interpolatedSample(
    Index channel,
    number index,
    SampleIndex end,
    SampleIndex length,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    SampleIndex index1 = (SampleIndex)(trunc(index));
    number i_x = index - index1;
    number i_1px = 1. + i_x;
    number i_1mx = 1. - i_x;
    number i_2mx = 2. - i_x;
    number i_a = i_1mx * i_2mx;
    number i_b = i_1px * i_x;
    number i_p1 = -.1666667 * i_a * i_x;
    number i_p2 = .5 * i_1px * i_a;
    number i_p3 = .5 * i_b * i_2mx;
    number i_p4 = -.1666667 * i_b * i_1mx;
    SampleIndex index2 = (SampleIndex)(index1 + 1);

    if (index2 >= end)
        index2 -= length;

    SampleIndex index3 = (SampleIndex)(index1 + 2);

    if (index3 >= end)
        index3 -= length;

    SampleIndex index4 = (SampleIndex)(index1 + 3);

    if (index4 >= end)
        index4 -= length;

    return this->groove_02_getSample(channel, index1, offset, bufferLength) * i_p1 + this->groove_02_getSample(channel, index2, offset, bufferLength) * i_p2 + this->groove_02_getSample(channel, index3, offset, bufferLength) * i_p3 + this->groove_02_getSample(channel, index4, offset, bufferLength) * i_p4;
}

number groove_02_crossfadedSample(
    SampleValue out,
    number readIndex,
    Index channel,
    number rate,
    number loopMin,
    number loopMax,
    number loopLength,
    number offset,
    number bufferLength
) {
    auto crossFadeStart1 = this->maximum(loopMin - this->groove_02_crossfadeInSamples, 0);
    auto crossFadeEnd1 = this->minimum(crossFadeStart1 + this->groove_02_crossfadeInSamples, bufferLength);
    number crossFadeStart2 = crossFadeStart1 + loopLength;
    auto crossFadeEnd2 = this->minimum(crossFadeEnd1 + loopLength, bufferLength);
    number crossFadeLength = crossFadeEnd2 - crossFadeStart2;

    if (crossFadeLength > 0) {
        crossFadeEnd1 = crossFadeStart1 + crossFadeLength;
        number diff = -1;
        number addFactor = 0;

        if (readIndex >= crossFadeStart2) {
            diff = readIndex - crossFadeStart2;
            addFactor = -1;
        } else if (readIndex < crossFadeEnd1) {
            diff = crossFadeEnd1 - readIndex + loopMax - crossFadeStart2;
            addFactor = 1;
        }

        if (diff >= 0) {
            number out2ReadIndex = readIndex + loopLength * addFactor;
            number out2 = (rate == 1 ? this->groove_02_getSample(channel, trunc(out2ReadIndex), offset, bufferLength) : this->groove_02_interpolatedSample(channel, out2ReadIndex, loopMax, loopLength, offset, bufferLength));
            number out2Factor = diff / crossFadeLength;
            number out1Factor = 1 - out2Factor;
            return out * out1Factor + out2 * out2Factor;
        }
    }

    return out;
}

void groove_02_dspsetup(bool force) {
    if ((bool)(this->groove_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->groove_02_crossfadeInSamples = this->mstosamps(this->groove_02_crossfade);
    this->groove_02_setupDone = true;
}

void toggle_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_02_value_number;
}

void toggle_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_02_value_number_set(preset["value"]);
}

void random_01_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_01_nz_state
    );
}

void random_01_nz_init() {
    this->random_01_nz_reset();
}

void random_01_nz_seed(number v) {
    xoshiro_reset(v, this->random_01_nz_state);
}

number random_01_nz_next() {
    return xoshiro_next(this->random_01_nz_state);
}

number line_01_valueAtTime(MillisecondTime time) {
    return this->line_01_startValue + this->line_01_slope * (time - this->line_01_startTime);
}

void line_01_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_01_grain;
    number nextValue;

    if (nextTime - this->line_01_startTime >= this->line_01__time || this->line_01_grain == 0) {
        nextTime = this->line_01_startTime + this->line_01__time;
        nextValue = this->line_01_currentTarget;
    } else {
        nextValue = this->line_01_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1214668403, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_01_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_01_startValue = this->line_01_valueAtTime(currentTime);
    this->line_01_startTime = currentTime;
    this->line_01_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1214668403, false);;
    number rise = target - this->line_01_startValue;
    this->line_01_slope = rise / time;
    this->line_01_scheduleNext();
}

bool line_01_isFinished(number value) {
    return value == this->line_01_currentTarget && this->currenttime() - this->line_01_startTime >= this->line_01__time;
}

void line_01_startPendingRamp() {
    if (this->line_01_pendingRamps->length < 2) {
        this->line_01_pendingRamps->length = 0;
        this->line_01__time = 0;
        this->line_01_time_set(0);
        this->line_01_end_bang();
        return;
    }

    if (this->line_01_pendingRamps->length > 1) {
        number target = this->line_01_pendingRamps->shift();
        this->line_01__time = this->line_01_pendingRamps->shift();
        this->line_01__time = (this->line_01__time < 0 ? 0 : this->line_01__time);
        this->line_01_startRamp(target, this->line_01__time);
    }
}

number gaintilde_02_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_02_ramp_d_prev);
    this->gaintilde_02_ramp_d_prev = x;
    return temp;
}

void gaintilde_02_ramp_d_dspsetup() {
    this->gaintilde_02_ramp_d_reset();
}

void gaintilde_02_ramp_d_reset() {
    this->gaintilde_02_ramp_d_prev = 0;
}

number gaintilde_02_ramp_next(number x, number up, number down) {
    if (this->gaintilde_02_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_02_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_02_ramp_index = _up;
            this->gaintilde_02_ramp_increment = (x - this->gaintilde_02_ramp_prev) / _up;
        } else if (x < this->gaintilde_02_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_02_ramp_index = _down;
            this->gaintilde_02_ramp_increment = (x - this->gaintilde_02_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_02_ramp_index > 0) {
        this->gaintilde_02_ramp_prev += this->gaintilde_02_ramp_increment;
        this->gaintilde_02_ramp_index -= 1;
    } else {
        this->gaintilde_02_ramp_prev = x;
    }

    return this->gaintilde_02_ramp_prev;
}

void gaintilde_02_ramp_reset() {
    this->gaintilde_02_ramp_prev = 0;
    this->gaintilde_02_ramp_index = 0;
    this->gaintilde_02_ramp_increment = 0;
    this->gaintilde_02_ramp_d_reset();
}

void gaintilde_02_init() {
    this->gaintilde_02_loginc = rnbo_log(1.072);
    this->gaintilde_02_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_02_loginc));
}

void gaintilde_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_02_value;
}

void gaintilde_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_02_value_set(preset["value"]);
}

void gaintilde_02_dspsetup(bool force) {
    if ((bool)(this->gaintilde_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_02_setupDone = true;
    this->gaintilde_02_ramp_d_dspsetup();
}

void numberobj_01_init() {
    this->numberobj_01_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-214"), 1, this->_currentTime);
}

void numberobj_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_01_value;
}

void numberobj_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_01_value_set(preset["value"]);
}

void message_02_init() {
    this->message_02_set_set({2000});
}

void message_03_init() {
    this->message_03_set_set({0});
}

number pan_tilde_02_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_02_func_reset() {}

number groove_03_getSample(
    Index channel,
    SampleIndex index,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    if (offset > 0) {
        index += offset;

        if (index >= bufferLength)
            index -= bufferLength;
    }

    return this->groove_03_buffer->getSample(channel, index);
}

number groove_03_interpolatedSample(
    Index channel,
    number index,
    SampleIndex end,
    SampleIndex length,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    SampleIndex index1 = (SampleIndex)(trunc(index));
    number i_x = index - index1;
    number i_1px = 1. + i_x;
    number i_1mx = 1. - i_x;
    number i_2mx = 2. - i_x;
    number i_a = i_1mx * i_2mx;
    number i_b = i_1px * i_x;
    number i_p1 = -.1666667 * i_a * i_x;
    number i_p2 = .5 * i_1px * i_a;
    number i_p3 = .5 * i_b * i_2mx;
    number i_p4 = -.1666667 * i_b * i_1mx;
    SampleIndex index2 = (SampleIndex)(index1 + 1);

    if (index2 >= end)
        index2 -= length;

    SampleIndex index3 = (SampleIndex)(index1 + 2);

    if (index3 >= end)
        index3 -= length;

    SampleIndex index4 = (SampleIndex)(index1 + 3);

    if (index4 >= end)
        index4 -= length;

    return this->groove_03_getSample(channel, index1, offset, bufferLength) * i_p1 + this->groove_03_getSample(channel, index2, offset, bufferLength) * i_p2 + this->groove_03_getSample(channel, index3, offset, bufferLength) * i_p3 + this->groove_03_getSample(channel, index4, offset, bufferLength) * i_p4;
}

number groove_03_crossfadedSample(
    SampleValue out,
    number readIndex,
    Index channel,
    number rate,
    number loopMin,
    number loopMax,
    number loopLength,
    number offset,
    number bufferLength
) {
    auto crossFadeStart1 = this->maximum(loopMin - this->groove_03_crossfadeInSamples, 0);
    auto crossFadeEnd1 = this->minimum(crossFadeStart1 + this->groove_03_crossfadeInSamples, bufferLength);
    number crossFadeStart2 = crossFadeStart1 + loopLength;
    auto crossFadeEnd2 = this->minimum(crossFadeEnd1 + loopLength, bufferLength);
    number crossFadeLength = crossFadeEnd2 - crossFadeStart2;

    if (crossFadeLength > 0) {
        crossFadeEnd1 = crossFadeStart1 + crossFadeLength;
        number diff = -1;
        number addFactor = 0;

        if (readIndex >= crossFadeStart2) {
            diff = readIndex - crossFadeStart2;
            addFactor = -1;
        } else if (readIndex < crossFadeEnd1) {
            diff = crossFadeEnd1 - readIndex + loopMax - crossFadeStart2;
            addFactor = 1;
        }

        if (diff >= 0) {
            number out2ReadIndex = readIndex + loopLength * addFactor;
            number out2 = (rate == 1 ? this->groove_03_getSample(channel, trunc(out2ReadIndex), offset, bufferLength) : this->groove_03_interpolatedSample(channel, out2ReadIndex, loopMax, loopLength, offset, bufferLength));
            number out2Factor = diff / crossFadeLength;
            number out1Factor = 1 - out2Factor;
            return out * out1Factor + out2 * out2Factor;
        }
    }

    return out;
}

void groove_03_dspsetup(bool force) {
    if ((bool)(this->groove_03_setupDone) && (bool)(!(bool)(force)))
        return;

    this->groove_03_crossfadeInSamples = this->mstosamps(this->groove_03_crossfade);
    this->groove_03_setupDone = true;
}

number pan_tilde_03_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_03_func_reset() {}

void adsr_01_dspsetup(bool force) {
    if ((bool)(this->adsr_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->adsr_01_mspersamp = (number)1000 / this->sr;
    this->adsr_01_setupDone = true;
}

void toggle_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_03_value_number;
}

void toggle_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_03_value_number_set(preset["value"]);
}

void numberobj_02_init() {
    this->numberobj_02_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-41"), 1, this->_currentTime);
}

void numberobj_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_02_value;
}

void numberobj_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_02_value_set(preset["value"]);
}

void counter_01_inc() {
    this->counter_01_count = ((bool)(this->counter_01_phase) ? this->counter_01_count + 1 : this->counter_01_count - 1);
}

void metro_01_onTransportChanged(number ) {}

void metro_01_onBeatTimeChanged(number ) {}

void toggle_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_04_value_number;
}

void toggle_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_04_value_number_set(preset["value"]);
}

void slider_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_01_value;
}

void slider_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_01_value_set(preset["value"]);
}

number mtof_tilde_01_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_tilde_01_innerMtoF_lastInValue && tuning == this->mtof_tilde_01_innerMtoF_lastTuning)
        return this->mtof_tilde_01_innerMtoF_lastOutValue;

    this->mtof_tilde_01_innerMtoF_lastInValue = midivalue;
    this->mtof_tilde_01_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_tilde_01_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_tilde_01_innerMtoF_lastOutValue;
}

void mtof_tilde_01_innerMtoF_reset() {
    this->mtof_tilde_01_innerMtoF_lastInValue = 0;
    this->mtof_tilde_01_innerMtoF_lastOutValue = 0;
    this->mtof_tilde_01_innerMtoF_lastTuning = 0;
}

void mtof_tilde_01_innerScala_mid(int v) {
    this->mtof_tilde_01_innerScala_kbmMid = v;
    this->mtof_tilde_01_innerScala_updateRefFreq();
}

void mtof_tilde_01_innerScala_ref(int v) {
    this->mtof_tilde_01_innerScala_kbmRefNum = v;
    this->mtof_tilde_01_innerScala_updateRefFreq();
}

void mtof_tilde_01_innerScala_base(number v) {
    this->mtof_tilde_01_innerScala_kbmRefFreq = v;
    this->mtof_tilde_01_innerScala_updateRefFreq();
}

void mtof_tilde_01_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_tilde_01_innerScala_updateScale(sclValid);
}

void mtof_tilde_01_innerScala_update(list scale, list map) {
    if (scale->length > 0) {
        this->mtof_tilde_01_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_tilde_01_innerScala_updateMap(map);
    }
}

number mtof_tilde_01_innerScala_mtof(number note) {
    if ((bool)(this->mtof_tilde_01_innerScala_lastValid) && this->mtof_tilde_01_innerScala_lastNote == note) {
        return this->mtof_tilde_01_innerScala_lastFreq;
    }

    array<int, 2> degoct = this->mtof_tilde_01_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_tilde_01_innerScala_applySCL(degoct[0], fract(note), this->mtof_tilde_01_innerScala_refFreq);
    }

    this->mtof_tilde_01_innerScala_updateLast(note, out);
    return out;
}

number mtof_tilde_01_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_tilde_01_innerScala_lastValid) && this->mtof_tilde_01_innerScala_lastFreq == hz) {
        return this->mtof_tilde_01_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_tilde_01_innerScala_hztodeg(hz);
    int degree = (int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_tilde_01_innerScala_kbmSize == 0) {
        out = this->mtof_tilde_01_innerScala_kbmMid + degree;
    } else {
        array<int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_tilde_01_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_tilde_01_innerScala_kbmValid[(Index)(i + this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_tilde_01_innerScala_kbmSize + entry + this->mtof_tilde_01_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_tilde_01_innerScala_updateLast(out, hz);
    return this->mtof_tilde_01_innerScala_lastNote;
}

int mtof_tilde_01_innerScala_updateScale(list scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_tilde_01_innerScala_lastValid = false;
        this->mtof_tilde_01_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_tilde_01_innerScala_sclExpMul->push(last);
        }

        this->mtof_tilde_01_innerScala_sclOctaveMul = last;
        this->mtof_tilde_01_innerScala_sclEntryCount = (int)(this->mtof_tilde_01_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_tilde_01_innerScala_kbmMid = (int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_tilde_01_innerScala_kbmRefNum = (int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_tilde_01_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_tilde_01_innerScala_kbmSize = (int)(0);
        }

        this->mtof_tilde_01_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

int mtof_tilde_01_innerScala_updateMap(list kbm) {
    if (kbm->length == 1 && kbm[0] == 0.0) {
        kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (kbm->length >= 6 && kbm[0] >= 0.0) {
        this->mtof_tilde_01_innerScala_lastValid = false;
        Index size = (Index)(kbm[0]);
        int octave = 12;

        if (kbm->length > 6) {
            octave = (int)(kbm[6]);
        }

        if (size > 0 && kbm->length < this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_tilde_01_innerScala_kbmSize = (int)(size);
        this->mtof_tilde_01_innerScala_kbmMin = (int)(kbm[1]);
        this->mtof_tilde_01_innerScala_kbmMax = (int)(kbm[2]);
        this->mtof_tilde_01_innerScala_kbmMid = (int)(kbm[3]);
        this->mtof_tilde_01_innerScala_kbmRefNum = (int)(kbm[4]);
        this->mtof_tilde_01_innerScala_kbmRefFreq = kbm[5];
        this->mtof_tilde_01_innerScala_kbmOctaveDegree = octave;
        this->mtof_tilde_01_innerScala_kbmValid = kbm;
        this->mtof_tilde_01_innerScala_kbmMapSize = (kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET > kbm->length ? kbm->length : (kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET < 0 ? 0 : kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET));
        this->mtof_tilde_01_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_tilde_01_innerScala_updateLast(number note, number freq) {
    this->mtof_tilde_01_innerScala_lastValid = true;
    this->mtof_tilde_01_innerScala_lastNote = note;
    this->mtof_tilde_01_innerScala_lastFreq = freq;
}

array<number, 2> mtof_tilde_01_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_tilde_01_innerScala_refFreq) / this->mtof_tilde_01_innerScala_sclOctaveMul
    );

    int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_tilde_01_innerScala_sclEntryCount; i++) {
        number c = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_tilde_01_innerScala_refFreq);
        n = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_tilde_01_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_tilde_01_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_tilde_01_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_tilde_01_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<int, 2> mtof_tilde_01_innerScala_octdegree(int degree, int count) {
    int octave = 0;
    int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<int, 2> mtof_tilde_01_innerScala_applyKBM(number note) {
    if ((this->mtof_tilde_01_innerScala_kbmMin == this->mtof_tilde_01_innerScala_kbmMax && this->mtof_tilde_01_innerScala_kbmMax == 0) || (note >= this->mtof_tilde_01_innerScala_kbmMin && note <= this->mtof_tilde_01_innerScala_kbmMax)) {
        int degree = (int)(rnbo_floor(note - this->mtof_tilde_01_innerScala_kbmMid));

        if (this->mtof_tilde_01_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_kbmSize);
        int octave = (int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_tilde_01_innerScala_kbmMapSize > index) {
            degree = (int)(this->mtof_tilde_01_innerScala_kbmValid[(Index)(this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_tilde_01_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_tilde_01_innerScala_applySCL(int degree, number frac, number refFreq) {
    array<int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_sclEntryCount);
    return this->mtof_tilde_01_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_tilde_01_innerScala_applySCLOctIndex(number octave, int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_tilde_01_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_tilde_01_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_tilde_01_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_tilde_01_innerScala_sclOctaveMul);
}

void mtof_tilde_01_innerScala_updateRefFreq() {
    this->mtof_tilde_01_innerScala_lastValid = false;
    int refOffset = (int)(this->mtof_tilde_01_innerScala_kbmRefNum - this->mtof_tilde_01_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_tilde_01_innerScala_refFreq = this->mtof_tilde_01_innerScala_kbmRefFreq;
    } else {
        int base = (int)(this->mtof_tilde_01_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_tilde_01_innerScala_sclEntryCount;
        }

        array<int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_tilde_01_innerScala_kbmSize) {
            if (index < this->mtof_tilde_01_innerScala_kbmMapSize) {
                index = (int)(this->mtof_tilde_01_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > this->mtof_tilde_01_innerScala_sclExpMul->length)
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_tilde_01_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_tilde_01_innerScala_refFreq = this->mtof_tilde_01_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_tilde_01_innerScala_sclOctaveMul);
        }
    }
}

void mtof_tilde_01_innerScala_reset() {
    this->mtof_tilde_01_innerScala_lastValid = false;
    this->mtof_tilde_01_innerScala_lastNote = 0;
    this->mtof_tilde_01_innerScala_lastFreq = 0;
    this->mtof_tilde_01_innerScala_sclEntryCount = 0;
    this->mtof_tilde_01_innerScala_sclOctaveMul = 1;
    this->mtof_tilde_01_innerScala_sclExpMul = {};
    this->mtof_tilde_01_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_tilde_01_innerScala_kbmMid = 60;
    this->mtof_tilde_01_innerScala_kbmRefNum = 69;
    this->mtof_tilde_01_innerScala_kbmRefFreq = 440;
    this->mtof_tilde_01_innerScala_kbmSize = 0;
    this->mtof_tilde_01_innerScala_kbmMin = 0;
    this->mtof_tilde_01_innerScala_kbmMax = 0;
    this->mtof_tilde_01_innerScala_kbmOctaveDegree = 12;
    this->mtof_tilde_01_innerScala_kbmMapSize = 0;
    this->mtof_tilde_01_innerScala_refFreq = 261.63;
}

void mtof_tilde_01_init() {
    this->mtof_tilde_01_innerScala_update(this->mtof_tilde_01_scale, this->mtof_tilde_01_map);
}

number cycle_tilde_01_ph_next(number freq, number reset) {
    {
        {
            if (reset >= 0.)
                this->cycle_tilde_01_ph_currentPhase = reset;
        }
    }

    number pincr = freq * this->cycle_tilde_01_ph_conv;

    if (this->cycle_tilde_01_ph_currentPhase < 0.)
        this->cycle_tilde_01_ph_currentPhase = 1. + this->cycle_tilde_01_ph_currentPhase;

    if (this->cycle_tilde_01_ph_currentPhase > 1.)
        this->cycle_tilde_01_ph_currentPhase = this->cycle_tilde_01_ph_currentPhase - 1.;

    number tmp = this->cycle_tilde_01_ph_currentPhase;
    this->cycle_tilde_01_ph_currentPhase += pincr;
    return tmp;
}

void cycle_tilde_01_ph_reset() {
    this->cycle_tilde_01_ph_currentPhase = 0;
}

void cycle_tilde_01_ph_dspsetup() {
    this->cycle_tilde_01_ph_conv = (number)1 / this->sr;
}

void cycle_tilde_01_dspsetup(bool force) {
    if ((bool)(this->cycle_tilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->cycle_tilde_01_phasei = 0;
    this->cycle_tilde_01_f2i = (number)4294967296 / this->samplerate();
    this->cycle_tilde_01_wrap = (long)(this->cycle_tilde_01_buffer->getSize()) - 1;
    this->cycle_tilde_01_setupDone = true;
    this->cycle_tilde_01_ph_dspsetup();
}

void cycle_tilde_01_bufferUpdated() {
    this->cycle_tilde_01_wrap = (long)(this->cycle_tilde_01_buffer->getSize()) - 1;
}

number line_02_valueAtTime(MillisecondTime time) {
    return this->line_02_startValue + this->line_02_slope * (time - this->line_02_startTime);
}

void line_02_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_02_grain;
    number nextValue;

    if (nextTime - this->line_02_startTime >= this->line_02__time || this->line_02_grain == 0) {
        nextTime = this->line_02_startTime + this->line_02__time;
        nextValue = this->line_02_currentTarget;
    } else {
        nextValue = this->line_02_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 1634454928, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_02_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_02_startValue = this->line_02_valueAtTime(currentTime);
    this->line_02_startTime = currentTime;
    this->line_02_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 1634454928, false);;
    number rise = target - this->line_02_startValue;
    this->line_02_slope = rise / time;
    this->line_02_scheduleNext();
}

bool line_02_isFinished(number value) {
    return value == this->line_02_currentTarget && this->currenttime() - this->line_02_startTime >= this->line_02__time;
}

void line_02_startPendingRamp() {
    if (this->line_02_pendingRamps->length < 2) {
        this->line_02_pendingRamps->length = 0;
        this->line_02__time = 0;
        this->line_02_time_set(0);
        this->line_02_end_bang();
        return;
    }

    if (this->line_02_pendingRamps->length > 1) {
        number target = this->line_02_pendingRamps->shift();
        this->line_02__time = this->line_02_pendingRamps->shift();
        this->line_02__time = (this->line_02__time < 0 ? 0 : this->line_02__time);
        this->line_02_startRamp(target, this->line_02__time);
    }
}

void message_04_init() {
    this->message_04_set_set({0, 2000, 45, 3000});
}

void slider_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_02_value;
}

void slider_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_02_value_set(preset["value"]);
}

void numberobj_03_init() {
    this->numberobj_03_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-30"), 1, this->_currentTime);
}

void numberobj_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_03_value;
}

void numberobj_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_03_value_set(preset["value"]);
}

number gaintilde_03_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_03_ramp_d_prev);
    this->gaintilde_03_ramp_d_prev = x;
    return temp;
}

void gaintilde_03_ramp_d_dspsetup() {
    this->gaintilde_03_ramp_d_reset();
}

void gaintilde_03_ramp_d_reset() {
    this->gaintilde_03_ramp_d_prev = 0;
}

number gaintilde_03_ramp_next(number x, number up, number down) {
    if (this->gaintilde_03_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_03_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_03_ramp_index = _up;
            this->gaintilde_03_ramp_increment = (x - this->gaintilde_03_ramp_prev) / _up;
        } else if (x < this->gaintilde_03_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_03_ramp_index = _down;
            this->gaintilde_03_ramp_increment = (x - this->gaintilde_03_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_03_ramp_index > 0) {
        this->gaintilde_03_ramp_prev += this->gaintilde_03_ramp_increment;
        this->gaintilde_03_ramp_index -= 1;
    } else {
        this->gaintilde_03_ramp_prev = x;
    }

    return this->gaintilde_03_ramp_prev;
}

void gaintilde_03_ramp_reset() {
    this->gaintilde_03_ramp_prev = 0;
    this->gaintilde_03_ramp_index = 0;
    this->gaintilde_03_ramp_increment = 0;
    this->gaintilde_03_ramp_d_reset();
}

void gaintilde_03_init() {
    this->gaintilde_03_loginc = rnbo_log(1.072);
    this->gaintilde_03_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_03_loginc));
}

void gaintilde_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_03_value;
}

void gaintilde_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_03_value_set(preset["value"]);
}

void gaintilde_03_dspsetup(bool force) {
    if ((bool)(this->gaintilde_03_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_03_setupDone = true;
    this->gaintilde_03_ramp_d_dspsetup();
}

void random_02_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_02_nz_state
    );
}

void random_02_nz_init() {
    this->random_02_nz_reset();
}

void random_02_nz_seed(number v) {
    xoshiro_reset(v, this->random_02_nz_state);
}

number random_02_nz_next() {
    return xoshiro_next(this->random_02_nz_state);
}

number line_03_valueAtTime(MillisecondTime time) {
    return this->line_03_startValue + this->line_03_slope * (time - this->line_03_startTime);
}

void line_03_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_03_grain;
    number nextValue;

    if (nextTime - this->line_03_startTime >= this->line_03__time || this->line_03_grain == 0) {
        nextTime = this->line_03_startTime + this->line_03__time;
        nextValue = this->line_03_currentTarget;
    } else {
        nextValue = this->line_03_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1586675634, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_03_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_03_startValue = this->line_03_valueAtTime(currentTime);
    this->line_03_startTime = currentTime;
    this->line_03_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1586675634, false);;
    number rise = target - this->line_03_startValue;
    this->line_03_slope = rise / time;
    this->line_03_scheduleNext();
}

bool line_03_isFinished(number value) {
    return value == this->line_03_currentTarget && this->currenttime() - this->line_03_startTime >= this->line_03__time;
}

void line_03_startPendingRamp() {
    if (this->line_03_pendingRamps->length < 2) {
        this->line_03_pendingRamps->length = 0;
        this->line_03__time = 0;
        this->line_03_time_set(0);
        this->line_03_end_bang();
        return;
    }

    if (this->line_03_pendingRamps->length > 1) {
        number target = this->line_03_pendingRamps->shift();
        this->line_03__time = this->line_03_pendingRamps->shift();
        this->line_03__time = (this->line_03__time < 0 ? 0 : this->line_03__time);
        this->line_03_startRamp(target, this->line_03__time);
    }
}

void message_05_init() {
    this->message_05_set_set({120});
}

void transport_01_onTempoChanged(number state) {
    this->transport_01_outtempo_set(state);
}

void transport_01_onTransportChanged(number state) {
    this->transport_01_state_set(state);
}

void transport_01_onTimeSignatureChanged(number numerator, number denominator) {
    this->transport_01_outtimesig_set({numerator, denominator});
}

number line_04_valueAtTime(MillisecondTime time) {
    return this->line_04_startValue + this->line_04_slope * (time - this->line_04_startTime);
}

void line_04_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_04_grain;
    number nextValue;

    if (nextTime - this->line_04_startTime >= this->line_04__time || this->line_04_grain == 0) {
        nextTime = this->line_04_startTime + this->line_04__time;
        nextValue = this->line_04_currentTarget;
    } else {
        nextValue = this->line_04_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 1262447697, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_04_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_04_startValue = this->line_04_valueAtTime(currentTime);
    this->line_04_startTime = currentTime;
    this->line_04_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 1262447697, false);;
    number rise = target - this->line_04_startValue;
    this->line_04_slope = rise / time;
    this->line_04_scheduleNext();
}

bool line_04_isFinished(number value) {
    return value == this->line_04_currentTarget && this->currenttime() - this->line_04_startTime >= this->line_04__time;
}

void line_04_startPendingRamp() {
    if (this->line_04_pendingRamps->length < 2) {
        this->line_04_pendingRamps->length = 0;
        this->line_04__time = 0;
        this->line_04_time_set(0);
        this->line_04_end_bang();
        return;
    }

    if (this->line_04_pendingRamps->length > 1) {
        number target = this->line_04_pendingRamps->shift();
        this->line_04__time = this->line_04_pendingRamps->shift();
        this->line_04__time = (this->line_04__time < 0 ? 0 : this->line_04__time);
        this->line_04_startRamp(target, this->line_04__time);
    }
}

void message_06_init() {
    this->message_06_set_set({0, 2000, 52, 3000});
}

void numberobj_04_init() {
    this->numberobj_04_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-244"), 1, this->_currentTime);
}

void numberobj_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_04_value;
}

void numberobj_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_04_value_set(preset["value"]);
}

number gaintilde_04_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_04_ramp_d_prev);
    this->gaintilde_04_ramp_d_prev = x;
    return temp;
}

void gaintilde_04_ramp_d_dspsetup() {
    this->gaintilde_04_ramp_d_reset();
}

void gaintilde_04_ramp_d_reset() {
    this->gaintilde_04_ramp_d_prev = 0;
}

number gaintilde_04_ramp_next(number x, number up, number down) {
    if (this->gaintilde_04_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_04_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_04_ramp_index = _up;
            this->gaintilde_04_ramp_increment = (x - this->gaintilde_04_ramp_prev) / _up;
        } else if (x < this->gaintilde_04_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_04_ramp_index = _down;
            this->gaintilde_04_ramp_increment = (x - this->gaintilde_04_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_04_ramp_index > 0) {
        this->gaintilde_04_ramp_prev += this->gaintilde_04_ramp_increment;
        this->gaintilde_04_ramp_index -= 1;
    } else {
        this->gaintilde_04_ramp_prev = x;
    }

    return this->gaintilde_04_ramp_prev;
}

void gaintilde_04_ramp_reset() {
    this->gaintilde_04_ramp_prev = 0;
    this->gaintilde_04_ramp_index = 0;
    this->gaintilde_04_ramp_increment = 0;
    this->gaintilde_04_ramp_d_reset();
}

void gaintilde_04_init() {
    this->gaintilde_04_loginc = rnbo_log(1.072);
    this->gaintilde_04_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_04_loginc));
}

void gaintilde_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_04_value;
}

void gaintilde_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_04_value_set(preset["value"]);
}

void gaintilde_04_dspsetup(bool force) {
    if ((bool)(this->gaintilde_04_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_04_setupDone = true;
    this->gaintilde_04_ramp_d_dspsetup();
}

void numberobj_05_init() {
    this->numberobj_05_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-212"), 1, this->_currentTime);
}

void numberobj_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_05_value;
}

void numberobj_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_05_value_set(preset["value"]);
}

void random_03_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_03_nz_state
    );
}

void random_03_nz_init() {
    this->random_03_nz_reset();
}

void random_03_nz_seed(number v) {
    xoshiro_reset(v, this->random_03_nz_state);
}

number random_03_nz_next() {
    return xoshiro_next(this->random_03_nz_state);
}

void slider_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_03_value;
}

void slider_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_03_value_set(preset["value"]);
}

number mtof_tilde_02_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_tilde_02_innerMtoF_lastInValue && tuning == this->mtof_tilde_02_innerMtoF_lastTuning)
        return this->mtof_tilde_02_innerMtoF_lastOutValue;

    this->mtof_tilde_02_innerMtoF_lastInValue = midivalue;
    this->mtof_tilde_02_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_tilde_02_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_tilde_02_innerMtoF_lastOutValue;
}

void mtof_tilde_02_innerMtoF_reset() {
    this->mtof_tilde_02_innerMtoF_lastInValue = 0;
    this->mtof_tilde_02_innerMtoF_lastOutValue = 0;
    this->mtof_tilde_02_innerMtoF_lastTuning = 0;
}

void mtof_tilde_02_innerScala_mid(int v) {
    this->mtof_tilde_02_innerScala_kbmMid = v;
    this->mtof_tilde_02_innerScala_updateRefFreq();
}

void mtof_tilde_02_innerScala_ref(int v) {
    this->mtof_tilde_02_innerScala_kbmRefNum = v;
    this->mtof_tilde_02_innerScala_updateRefFreq();
}

void mtof_tilde_02_innerScala_base(number v) {
    this->mtof_tilde_02_innerScala_kbmRefFreq = v;
    this->mtof_tilde_02_innerScala_updateRefFreq();
}

void mtof_tilde_02_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_tilde_02_innerScala_updateScale(sclValid);
}

void mtof_tilde_02_innerScala_update(list scale, list map) {
    if (scale->length > 0) {
        this->mtof_tilde_02_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_tilde_02_innerScala_updateMap(map);
    }
}

number mtof_tilde_02_innerScala_mtof(number note) {
    if ((bool)(this->mtof_tilde_02_innerScala_lastValid) && this->mtof_tilde_02_innerScala_lastNote == note) {
        return this->mtof_tilde_02_innerScala_lastFreq;
    }

    array<int, 2> degoct = this->mtof_tilde_02_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_tilde_02_innerScala_applySCL(degoct[0], fract(note), this->mtof_tilde_02_innerScala_refFreq);
    }

    this->mtof_tilde_02_innerScala_updateLast(note, out);
    return out;
}

number mtof_tilde_02_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_tilde_02_innerScala_lastValid) && this->mtof_tilde_02_innerScala_lastFreq == hz) {
        return this->mtof_tilde_02_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_tilde_02_innerScala_hztodeg(hz);
    int degree = (int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_tilde_02_innerScala_kbmSize == 0) {
        out = this->mtof_tilde_02_innerScala_kbmMid + degree;
    } else {
        array<int, 2> octdeg = this->mtof_tilde_02_innerScala_octdegree(degree, this->mtof_tilde_02_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_tilde_02_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_tilde_02_innerScala_kbmValid[(Index)(i + this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_tilde_02_innerScala_kbmSize + entry + this->mtof_tilde_02_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_tilde_02_innerScala_updateLast(out, hz);
    return this->mtof_tilde_02_innerScala_lastNote;
}

int mtof_tilde_02_innerScala_updateScale(list scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_tilde_02_innerScala_lastValid = false;
        this->mtof_tilde_02_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_tilde_02_innerScala_sclExpMul->push(last);
        }

        this->mtof_tilde_02_innerScala_sclOctaveMul = last;
        this->mtof_tilde_02_innerScala_sclEntryCount = (int)(this->mtof_tilde_02_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_tilde_02_innerScala_kbmMid = (int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_tilde_02_innerScala_kbmRefNum = (int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_tilde_02_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_tilde_02_innerScala_kbmSize = (int)(0);
        }

        this->mtof_tilde_02_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

int mtof_tilde_02_innerScala_updateMap(list kbm) {
    if (kbm->length == 1 && kbm[0] == 0.0) {
        kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (kbm->length >= 6 && kbm[0] >= 0.0) {
        this->mtof_tilde_02_innerScala_lastValid = false;
        Index size = (Index)(kbm[0]);
        int octave = 12;

        if (kbm->length > 6) {
            octave = (int)(kbm[6]);
        }

        if (size > 0 && kbm->length < this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_tilde_02_innerScala_kbmSize = (int)(size);
        this->mtof_tilde_02_innerScala_kbmMin = (int)(kbm[1]);
        this->mtof_tilde_02_innerScala_kbmMax = (int)(kbm[2]);
        this->mtof_tilde_02_innerScala_kbmMid = (int)(kbm[3]);
        this->mtof_tilde_02_innerScala_kbmRefNum = (int)(kbm[4]);
        this->mtof_tilde_02_innerScala_kbmRefFreq = kbm[5];
        this->mtof_tilde_02_innerScala_kbmOctaveDegree = octave;
        this->mtof_tilde_02_innerScala_kbmValid = kbm;
        this->mtof_tilde_02_innerScala_kbmMapSize = (kbm->length - this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET > kbm->length ? kbm->length : (kbm->length - this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET < 0 ? 0 : kbm->length - this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET));
        this->mtof_tilde_02_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_tilde_02_innerScala_updateLast(number note, number freq) {
    this->mtof_tilde_02_innerScala_lastValid = true;
    this->mtof_tilde_02_innerScala_lastNote = note;
    this->mtof_tilde_02_innerScala_lastFreq = freq;
}

array<number, 2> mtof_tilde_02_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_tilde_02_innerScala_refFreq) / this->mtof_tilde_02_innerScala_sclOctaveMul
    );

    int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_tilde_02_innerScala_sclEntryCount; i++) {
        number c = this->mtof_tilde_02_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_tilde_02_innerScala_refFreq);
        n = this->mtof_tilde_02_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_tilde_02_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_tilde_02_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_tilde_02_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_tilde_02_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_tilde_02_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<int, 2> mtof_tilde_02_innerScala_octdegree(int degree, int count) {
    int octave = 0;
    int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<int, 2> mtof_tilde_02_innerScala_applyKBM(number note) {
    if ((this->mtof_tilde_02_innerScala_kbmMin == this->mtof_tilde_02_innerScala_kbmMax && this->mtof_tilde_02_innerScala_kbmMax == 0) || (note >= this->mtof_tilde_02_innerScala_kbmMin && note <= this->mtof_tilde_02_innerScala_kbmMax)) {
        int degree = (int)(rnbo_floor(note - this->mtof_tilde_02_innerScala_kbmMid));

        if (this->mtof_tilde_02_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<int, 2> octdeg = this->mtof_tilde_02_innerScala_octdegree(degree, this->mtof_tilde_02_innerScala_kbmSize);
        int octave = (int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_tilde_02_innerScala_kbmMapSize > index) {
            degree = (int)(this->mtof_tilde_02_innerScala_kbmValid[(Index)(this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_tilde_02_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_tilde_02_innerScala_applySCL(int degree, number frac, number refFreq) {
    array<int, 2> octdeg = this->mtof_tilde_02_innerScala_octdegree(degree, this->mtof_tilde_02_innerScala_sclEntryCount);
    return this->mtof_tilde_02_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_tilde_02_innerScala_applySCLOctIndex(number octave, int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_tilde_02_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_tilde_02_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_tilde_02_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_tilde_02_innerScala_sclOctaveMul);
}

void mtof_tilde_02_innerScala_updateRefFreq() {
    this->mtof_tilde_02_innerScala_lastValid = false;
    int refOffset = (int)(this->mtof_tilde_02_innerScala_kbmRefNum - this->mtof_tilde_02_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_tilde_02_innerScala_refFreq = this->mtof_tilde_02_innerScala_kbmRefFreq;
    } else {
        int base = (int)(this->mtof_tilde_02_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_tilde_02_innerScala_sclEntryCount;
        }

        array<int, 2> octdeg = this->mtof_tilde_02_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_tilde_02_innerScala_kbmSize) {
            if (index < this->mtof_tilde_02_innerScala_kbmMapSize) {
                index = (int)(this->mtof_tilde_02_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_tilde_02_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > this->mtof_tilde_02_innerScala_sclExpMul->length)
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_tilde_02_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_tilde_02_innerScala_refFreq = this->mtof_tilde_02_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_tilde_02_innerScala_sclOctaveMul);
        }
    }
}

void mtof_tilde_02_innerScala_reset() {
    this->mtof_tilde_02_innerScala_lastValid = false;
    this->mtof_tilde_02_innerScala_lastNote = 0;
    this->mtof_tilde_02_innerScala_lastFreq = 0;
    this->mtof_tilde_02_innerScala_sclEntryCount = 0;
    this->mtof_tilde_02_innerScala_sclOctaveMul = 1;
    this->mtof_tilde_02_innerScala_sclExpMul = {};
    this->mtof_tilde_02_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_tilde_02_innerScala_kbmMid = 60;
    this->mtof_tilde_02_innerScala_kbmRefNum = 69;
    this->mtof_tilde_02_innerScala_kbmRefFreq = 440;
    this->mtof_tilde_02_innerScala_kbmSize = 0;
    this->mtof_tilde_02_innerScala_kbmMin = 0;
    this->mtof_tilde_02_innerScala_kbmMax = 0;
    this->mtof_tilde_02_innerScala_kbmOctaveDegree = 12;
    this->mtof_tilde_02_innerScala_kbmMapSize = 0;
    this->mtof_tilde_02_innerScala_refFreq = 261.63;
}

void mtof_tilde_02_init() {
    this->mtof_tilde_02_innerScala_update(this->mtof_tilde_02_scale, this->mtof_tilde_02_map);
}

number rect_tilde_01_rectangle(number phase, number pulsewidth) {
    if (phase < pulsewidth) {
        return 1;
    } else {
        return -1;
    }
}

void message_07_init() {
    this->message_07_set_set({2000});
}

void slider_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_04_value;
}

void slider_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_04_value_set(preset["value"]);
}

number line_05_valueAtTime(MillisecondTime time) {
    return this->line_05_startValue + this->line_05_slope * (time - this->line_05_startTime);
}

void line_05_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_05_grain;
    number nextValue;

    if (nextTime - this->line_05_startTime >= this->line_05__time || this->line_05_grain == 0) {
        nextTime = this->line_05_startTime + this->line_05__time;
        nextValue = this->line_05_currentTarget;
    } else {
        nextValue = this->line_05_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 890440466, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_05_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_05_startValue = this->line_05_valueAtTime(currentTime);
    this->line_05_startTime = currentTime;
    this->line_05_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 890440466, false);;
    number rise = target - this->line_05_startValue;
    this->line_05_slope = rise / time;
    this->line_05_scheduleNext();
}

bool line_05_isFinished(number value) {
    return value == this->line_05_currentTarget && this->currenttime() - this->line_05_startTime >= this->line_05__time;
}

void line_05_startPendingRamp() {
    if (this->line_05_pendingRamps->length < 2) {
        this->line_05_pendingRamps->length = 0;
        this->line_05__time = 0;
        this->line_05_time_set(0);
        this->line_05_end_bang();
        return;
    }

    if (this->line_05_pendingRamps->length > 1) {
        number target = this->line_05_pendingRamps->shift();
        this->line_05__time = this->line_05_pendingRamps->shift();
        this->line_05__time = (this->line_05__time < 0 ? 0 : this->line_05__time);
        this->line_05_startRamp(target, this->line_05__time);
    }
}

void message_08_init() {
    this->message_08_set_set({0, 2000, 57, 3000});
}

void numberobj_06_init() {
    this->numberobj_06_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-31"), 1, this->_currentTime);
}

void numberobj_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_06_value;
}

void numberobj_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_06_value_set(preset["value"]);
}

number line_06_valueAtTime(MillisecondTime time) {
    return this->line_06_startValue + this->line_06_slope * (time - this->line_06_startTime);
}

void line_06_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_06_grain;
    number nextValue;

    if (nextTime - this->line_06_startTime >= this->line_06__time || this->line_06_grain == 0) {
        nextTime = this->line_06_startTime + this->line_06__time;
        nextValue = this->line_06_currentTarget;
    } else {
        nextValue = this->line_06_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 518433235, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_06_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_06_startValue = this->line_06_valueAtTime(currentTime);
    this->line_06_startTime = currentTime;
    this->line_06_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 518433235, false);;
    number rise = target - this->line_06_startValue;
    this->line_06_slope = rise / time;
    this->line_06_scheduleNext();
}

bool line_06_isFinished(number value) {
    return value == this->line_06_currentTarget && this->currenttime() - this->line_06_startTime >= this->line_06__time;
}

void line_06_startPendingRamp() {
    if (this->line_06_pendingRamps->length < 2) {
        this->line_06_pendingRamps->length = 0;
        this->line_06__time = 0;
        this->line_06_time_set(0);
        this->line_06_end_bang();
        return;
    }

    if (this->line_06_pendingRamps->length > 1) {
        number target = this->line_06_pendingRamps->shift();
        this->line_06__time = this->line_06_pendingRamps->shift();
        this->line_06__time = (this->line_06__time < 0 ? 0 : this->line_06__time);
        this->line_06_startRamp(target, this->line_06__time);
    }
}

void message_09_init() {
    this->message_09_set_set({0, 2000, 57, 3000});
}

number pan_tilde_04_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_04_func_reset() {}

void slider_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_05_value;
}

void slider_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_05_value_set(preset["value"]);
}

number mtof_tilde_03_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_tilde_03_innerMtoF_lastInValue && tuning == this->mtof_tilde_03_innerMtoF_lastTuning)
        return this->mtof_tilde_03_innerMtoF_lastOutValue;

    this->mtof_tilde_03_innerMtoF_lastInValue = midivalue;
    this->mtof_tilde_03_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_tilde_03_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_tilde_03_innerMtoF_lastOutValue;
}

void mtof_tilde_03_innerMtoF_reset() {
    this->mtof_tilde_03_innerMtoF_lastInValue = 0;
    this->mtof_tilde_03_innerMtoF_lastOutValue = 0;
    this->mtof_tilde_03_innerMtoF_lastTuning = 0;
}

void mtof_tilde_03_innerScala_mid(int v) {
    this->mtof_tilde_03_innerScala_kbmMid = v;
    this->mtof_tilde_03_innerScala_updateRefFreq();
}

void mtof_tilde_03_innerScala_ref(int v) {
    this->mtof_tilde_03_innerScala_kbmRefNum = v;
    this->mtof_tilde_03_innerScala_updateRefFreq();
}

void mtof_tilde_03_innerScala_base(number v) {
    this->mtof_tilde_03_innerScala_kbmRefFreq = v;
    this->mtof_tilde_03_innerScala_updateRefFreq();
}

void mtof_tilde_03_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_tilde_03_innerScala_updateScale(sclValid);
}

void mtof_tilde_03_innerScala_update(list scale, list map) {
    if (scale->length > 0) {
        this->mtof_tilde_03_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_tilde_03_innerScala_updateMap(map);
    }
}

number mtof_tilde_03_innerScala_mtof(number note) {
    if ((bool)(this->mtof_tilde_03_innerScala_lastValid) && this->mtof_tilde_03_innerScala_lastNote == note) {
        return this->mtof_tilde_03_innerScala_lastFreq;
    }

    array<int, 2> degoct = this->mtof_tilde_03_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_tilde_03_innerScala_applySCL(degoct[0], fract(note), this->mtof_tilde_03_innerScala_refFreq);
    }

    this->mtof_tilde_03_innerScala_updateLast(note, out);
    return out;
}

number mtof_tilde_03_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_tilde_03_innerScala_lastValid) && this->mtof_tilde_03_innerScala_lastFreq == hz) {
        return this->mtof_tilde_03_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_tilde_03_innerScala_hztodeg(hz);
    int degree = (int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_tilde_03_innerScala_kbmSize == 0) {
        out = this->mtof_tilde_03_innerScala_kbmMid + degree;
    } else {
        array<int, 2> octdeg = this->mtof_tilde_03_innerScala_octdegree(degree, this->mtof_tilde_03_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_tilde_03_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_tilde_03_innerScala_kbmValid[(Index)(i + this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_tilde_03_innerScala_kbmSize + entry + this->mtof_tilde_03_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_tilde_03_innerScala_updateLast(out, hz);
    return this->mtof_tilde_03_innerScala_lastNote;
}

int mtof_tilde_03_innerScala_updateScale(list scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_tilde_03_innerScala_lastValid = false;
        this->mtof_tilde_03_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_tilde_03_innerScala_sclExpMul->push(last);
        }

        this->mtof_tilde_03_innerScala_sclOctaveMul = last;
        this->mtof_tilde_03_innerScala_sclEntryCount = (int)(this->mtof_tilde_03_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_tilde_03_innerScala_kbmMid = (int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_tilde_03_innerScala_kbmRefNum = (int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_tilde_03_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_tilde_03_innerScala_kbmSize = (int)(0);
        }

        this->mtof_tilde_03_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

int mtof_tilde_03_innerScala_updateMap(list kbm) {
    if (kbm->length == 1 && kbm[0] == 0.0) {
        kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (kbm->length >= 6 && kbm[0] >= 0.0) {
        this->mtof_tilde_03_innerScala_lastValid = false;
        Index size = (Index)(kbm[0]);
        int octave = 12;

        if (kbm->length > 6) {
            octave = (int)(kbm[6]);
        }

        if (size > 0 && kbm->length < this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_tilde_03_innerScala_kbmSize = (int)(size);
        this->mtof_tilde_03_innerScala_kbmMin = (int)(kbm[1]);
        this->mtof_tilde_03_innerScala_kbmMax = (int)(kbm[2]);
        this->mtof_tilde_03_innerScala_kbmMid = (int)(kbm[3]);
        this->mtof_tilde_03_innerScala_kbmRefNum = (int)(kbm[4]);
        this->mtof_tilde_03_innerScala_kbmRefFreq = kbm[5];
        this->mtof_tilde_03_innerScala_kbmOctaveDegree = octave;
        this->mtof_tilde_03_innerScala_kbmValid = kbm;
        this->mtof_tilde_03_innerScala_kbmMapSize = (kbm->length - this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET > kbm->length ? kbm->length : (kbm->length - this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET < 0 ? 0 : kbm->length - this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET));
        this->mtof_tilde_03_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_tilde_03_innerScala_updateLast(number note, number freq) {
    this->mtof_tilde_03_innerScala_lastValid = true;
    this->mtof_tilde_03_innerScala_lastNote = note;
    this->mtof_tilde_03_innerScala_lastFreq = freq;
}

array<number, 2> mtof_tilde_03_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_tilde_03_innerScala_refFreq) / this->mtof_tilde_03_innerScala_sclOctaveMul
    );

    int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_tilde_03_innerScala_sclEntryCount; i++) {
        number c = this->mtof_tilde_03_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_tilde_03_innerScala_refFreq);
        n = this->mtof_tilde_03_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_tilde_03_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_tilde_03_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_tilde_03_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_tilde_03_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_tilde_03_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<int, 2> mtof_tilde_03_innerScala_octdegree(int degree, int count) {
    int octave = 0;
    int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<int, 2> mtof_tilde_03_innerScala_applyKBM(number note) {
    if ((this->mtof_tilde_03_innerScala_kbmMin == this->mtof_tilde_03_innerScala_kbmMax && this->mtof_tilde_03_innerScala_kbmMax == 0) || (note >= this->mtof_tilde_03_innerScala_kbmMin && note <= this->mtof_tilde_03_innerScala_kbmMax)) {
        int degree = (int)(rnbo_floor(note - this->mtof_tilde_03_innerScala_kbmMid));

        if (this->mtof_tilde_03_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<int, 2> octdeg = this->mtof_tilde_03_innerScala_octdegree(degree, this->mtof_tilde_03_innerScala_kbmSize);
        int octave = (int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_tilde_03_innerScala_kbmMapSize > index) {
            degree = (int)(this->mtof_tilde_03_innerScala_kbmValid[(Index)(this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_tilde_03_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_tilde_03_innerScala_applySCL(int degree, number frac, number refFreq) {
    array<int, 2> octdeg = this->mtof_tilde_03_innerScala_octdegree(degree, this->mtof_tilde_03_innerScala_sclEntryCount);
    return this->mtof_tilde_03_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_tilde_03_innerScala_applySCLOctIndex(number octave, int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_tilde_03_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_tilde_03_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_tilde_03_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_tilde_03_innerScala_sclOctaveMul);
}

void mtof_tilde_03_innerScala_updateRefFreq() {
    this->mtof_tilde_03_innerScala_lastValid = false;
    int refOffset = (int)(this->mtof_tilde_03_innerScala_kbmRefNum - this->mtof_tilde_03_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_tilde_03_innerScala_refFreq = this->mtof_tilde_03_innerScala_kbmRefFreq;
    } else {
        int base = (int)(this->mtof_tilde_03_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_tilde_03_innerScala_sclEntryCount;
        }

        array<int, 2> octdeg = this->mtof_tilde_03_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_tilde_03_innerScala_kbmSize) {
            if (index < this->mtof_tilde_03_innerScala_kbmMapSize) {
                index = (int)(this->mtof_tilde_03_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_tilde_03_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > this->mtof_tilde_03_innerScala_sclExpMul->length)
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_tilde_03_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_tilde_03_innerScala_refFreq = this->mtof_tilde_03_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_tilde_03_innerScala_sclOctaveMul);
        }
    }
}

void mtof_tilde_03_innerScala_reset() {
    this->mtof_tilde_03_innerScala_lastValid = false;
    this->mtof_tilde_03_innerScala_lastNote = 0;
    this->mtof_tilde_03_innerScala_lastFreq = 0;
    this->mtof_tilde_03_innerScala_sclEntryCount = 0;
    this->mtof_tilde_03_innerScala_sclOctaveMul = 1;
    this->mtof_tilde_03_innerScala_sclExpMul = {};
    this->mtof_tilde_03_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_tilde_03_innerScala_kbmMid = 60;
    this->mtof_tilde_03_innerScala_kbmRefNum = 69;
    this->mtof_tilde_03_innerScala_kbmRefFreq = 440;
    this->mtof_tilde_03_innerScala_kbmSize = 0;
    this->mtof_tilde_03_innerScala_kbmMin = 0;
    this->mtof_tilde_03_innerScala_kbmMax = 0;
    this->mtof_tilde_03_innerScala_kbmOctaveDegree = 12;
    this->mtof_tilde_03_innerScala_kbmMapSize = 0;
    this->mtof_tilde_03_innerScala_refFreq = 261.63;
}

void mtof_tilde_03_init() {
    this->mtof_tilde_03_innerScala_update(this->mtof_tilde_03_scale, this->mtof_tilde_03_map);
}

number saw_tilde_01_dcblocker_next(number x, number gain) {
    number y = x - this->saw_tilde_01_dcblocker_xm1 + this->saw_tilde_01_dcblocker_ym1 * gain;
    this->saw_tilde_01_dcblocker_xm1 = x;
    this->saw_tilde_01_dcblocker_ym1 = y;
    return y;
}

void saw_tilde_01_dcblocker_reset() {
    this->saw_tilde_01_dcblocker_xm1 = 0;
    this->saw_tilde_01_dcblocker_ym1 = 0;
}

void saw_tilde_01_dcblocker_dspsetup() {
    this->saw_tilde_01_dcblocker_reset();
}

void saw_tilde_01_dspsetup(bool force) {
    if ((bool)(this->saw_tilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->saw_tilde_01_setupDone = true;
    this->saw_tilde_01_dcblocker_dspsetup();
}

void slider_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_06_value;
}

void slider_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_06_value_set(preset["value"]);
}

void message_10_init() {
    this->message_10_set_set({0.5, 3000, 0.8, 8000});
}

number line_07_valueAtTime(MillisecondTime time) {
    return this->line_07_startValue + this->line_07_slope * (time - this->line_07_startTime);
}

void line_07_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_07_grain;
    number nextValue;

    if (nextTime - this->line_07_startTime >= this->line_07__time || this->line_07_grain == 0) {
        nextTime = this->line_07_startTime + this->line_07__time;
        nextValue = this->line_07_currentTarget;
    } else {
        nextValue = this->line_07_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1172483444, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_07_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_07_startValue = this->line_07_valueAtTime(currentTime);
    this->line_07_startTime = currentTime;
    this->line_07_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1172483444, false);;
    number rise = target - this->line_07_startValue;
    this->line_07_slope = rise / time;
    this->line_07_scheduleNext();
}

bool line_07_isFinished(number value) {
    return value == this->line_07_currentTarget && this->currenttime() - this->line_07_startTime >= this->line_07__time;
}

void line_07_startPendingRamp() {
    if (this->line_07_pendingRamps->length < 2) {
        this->line_07_pendingRamps->length = 0;
        this->line_07__time = 0;
        this->line_07_time_set(0);
        this->line_07_end_bang();
        return;
    }

    if (this->line_07_pendingRamps->length > 1) {
        number target = this->line_07_pendingRamps->shift();
        this->line_07__time = this->line_07_pendingRamps->shift();
        this->line_07__time = (this->line_07__time < 0 ? 0 : this->line_07__time);
        this->line_07_startRamp(target, this->line_07__time);
    }
}

void message_11_init() {
    this->message_11_set_set({0, 1000, 145, 2000});
}

number line_08_valueAtTime(MillisecondTime time) {
    return this->line_08_startValue + this->line_08_slope * (time - this->line_08_startTime);
}

void line_08_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_08_grain;
    number nextValue;

    if (nextTime - this->line_08_startTime >= this->line_08__time || this->line_08_grain == 0) {
        nextTime = this->line_08_startTime + this->line_08__time;
        nextValue = this->line_08_currentTarget;
    } else {
        nextValue = this->line_08_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 1592269969, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_08_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_08_startValue = this->line_08_valueAtTime(currentTime);
    this->line_08_startTime = currentTime;
    this->line_08_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 1592269969, false);;
    number rise = target - this->line_08_startValue;
    this->line_08_slope = rise / time;
    this->line_08_scheduleNext();
}

bool line_08_isFinished(number value) {
    return value == this->line_08_currentTarget && this->currenttime() - this->line_08_startTime >= this->line_08__time;
}

void line_08_startPendingRamp() {
    if (this->line_08_pendingRamps->length < 2) {
        this->line_08_pendingRamps->length = 0;
        this->line_08__time = 0;
        this->line_08_time_set(0);
        this->line_08_end_bang();
        return;
    }

    if (this->line_08_pendingRamps->length > 1) {
        number target = this->line_08_pendingRamps->shift();
        this->line_08__time = this->line_08_pendingRamps->shift();
        this->line_08__time = (this->line_08__time < 0 ? 0 : this->line_08__time);
        this->line_08_startRamp(target, this->line_08__time);
    }
}

void param_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_02_value;
}

void param_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_02_value_set(preset["value"]);
}

void random_04_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_04_nz_state
    );
}

void random_04_nz_init() {
    this->random_04_nz_reset();
}

void random_04_nz_seed(number v) {
    xoshiro_reset(v, this->random_04_nz_state);
}

number random_04_nz_next() {
    return xoshiro_next(this->random_04_nz_state);
}

void numberobj_07_init() {
    this->numberobj_07_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-34"), 1, this->_currentTime);
}

void numberobj_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_07_value;
}

void numberobj_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_07_value_set(preset["value"]);
}

void random_05_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_05_nz_state
    );
}

void random_05_nz_init() {
    this->random_05_nz_reset();
}

void random_05_nz_seed(number v) {
    xoshiro_reset(v, this->random_05_nz_state);
}

number random_05_nz_next() {
    return xoshiro_next(this->random_05_nz_state);
}

number line_09_valueAtTime(MillisecondTime time) {
    return this->line_09_startValue + this->line_09_slope * (time - this->line_09_startTime);
}

void line_09_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_09_grain;
    number nextValue;

    if (nextTime - this->line_09_startTime >= this->line_09__time || this->line_09_grain == 0) {
        nextTime = this->line_09_startTime + this->line_09__time;
        nextValue = this->line_09_currentTarget;
    } else {
        nextValue = this->line_09_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1958682865, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_09_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_09_startValue = this->line_09_valueAtTime(currentTime);
    this->line_09_startTime = currentTime;
    this->line_09_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1958682865, false);;
    number rise = target - this->line_09_startValue;
    this->line_09_slope = rise / time;
    this->line_09_scheduleNext();
}

bool line_09_isFinished(number value) {
    return value == this->line_09_currentTarget && this->currenttime() - this->line_09_startTime >= this->line_09__time;
}

void line_09_startPendingRamp() {
    if (this->line_09_pendingRamps->length < 2) {
        this->line_09_pendingRamps->length = 0;
        this->line_09__time = 0;
        this->line_09_time_set(0);
        this->line_09_end_bang();
        return;
    }

    if (this->line_09_pendingRamps->length > 1) {
        number target = this->line_09_pendingRamps->shift();
        this->line_09__time = this->line_09_pendingRamps->shift();
        this->line_09__time = (this->line_09__time < 0 ? 0 : this->line_09__time);
        this->line_09_startRamp(target, this->line_09__time);
    }
}

void message_12_init() {
    this->message_12_set_set({0.5, 3000, 0.5, 8000});
}

number line_10_valueAtTime(MillisecondTime time) {
    return this->line_10_startValue + this->line_10_slope * (time - this->line_10_startTime);
}

void line_10_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_10_grain;
    number nextValue;

    if (nextTime - this->line_10_startTime >= this->line_10__time || this->line_10_grain == 0) {
        nextTime = this->line_10_startTime + this->line_10__time;
        nextValue = this->line_10_currentTarget;
    } else {
        nextValue = this->line_10_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1544490675, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_10_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_10_startValue = this->line_10_valueAtTime(currentTime);
    this->line_10_startTime = currentTime;
    this->line_10_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1544490675, false);;
    number rise = target - this->line_10_startValue;
    this->line_10_slope = rise / time;
    this->line_10_scheduleNext();
}

bool line_10_isFinished(number value) {
    return value == this->line_10_currentTarget && this->currenttime() - this->line_10_startTime >= this->line_10__time;
}

void line_10_startPendingRamp() {
    if (this->line_10_pendingRamps->length < 2) {
        this->line_10_pendingRamps->length = 0;
        this->line_10__time = 0;
        this->line_10_time_set(0);
        this->line_10_end_bang();
        return;
    }

    if (this->line_10_pendingRamps->length > 1) {
        number target = this->line_10_pendingRamps->shift();
        this->line_10__time = this->line_10_pendingRamps->shift();
        this->line_10__time = (this->line_10__time < 0 ? 0 : this->line_10__time);
        this->line_10_startRamp(target, this->line_10__time);
    }
}

void numberobj_08_init() {
    this->numberobj_08_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-90"), 1, this->_currentTime);
}

void numberobj_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_08_value;
}

void numberobj_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_08_value_set(preset["value"]);
}

void numberobj_09_init() {
    this->numberobj_09_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-253"), 1, this->_currentTime);
}

void numberobj_09_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_09_value;
}

void numberobj_09_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_09_value_set(preset["value"]);
}

number phasor_01_ph_next(number freq, number reset) {
    RNBO_UNUSED(reset);
    number pincr = freq * this->phasor_01_ph_conv;

    if (this->phasor_01_ph_currentPhase < 0.)
        this->phasor_01_ph_currentPhase = 1. + this->phasor_01_ph_currentPhase;

    if (this->phasor_01_ph_currentPhase > 1.)
        this->phasor_01_ph_currentPhase = this->phasor_01_ph_currentPhase - 1.;

    number tmp = this->phasor_01_ph_currentPhase;
    this->phasor_01_ph_currentPhase += pincr;
    return tmp;
}

void phasor_01_ph_reset() {
    this->phasor_01_ph_currentPhase = 0;
}

void phasor_01_ph_dspsetup() {
    this->phasor_01_ph_conv = (number)1 / this->sr;
}

void phasor_01_dspsetup(bool force) {
    if ((bool)(this->phasor_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->phasor_01_conv = (number)1 / this->samplerate();
    this->phasor_01_setupDone = true;
    this->phasor_01_ph_dspsetup();
}

void slider_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_07_value;
}

void slider_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_07_value_set(preset["value"]);
}

number mtof_tilde_04_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_tilde_04_innerMtoF_lastInValue && tuning == this->mtof_tilde_04_innerMtoF_lastTuning)
        return this->mtof_tilde_04_innerMtoF_lastOutValue;

    this->mtof_tilde_04_innerMtoF_lastInValue = midivalue;
    this->mtof_tilde_04_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_tilde_04_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_tilde_04_innerMtoF_lastOutValue;
}

void mtof_tilde_04_innerMtoF_reset() {
    this->mtof_tilde_04_innerMtoF_lastInValue = 0;
    this->mtof_tilde_04_innerMtoF_lastOutValue = 0;
    this->mtof_tilde_04_innerMtoF_lastTuning = 0;
}

void mtof_tilde_04_innerScala_mid(int v) {
    this->mtof_tilde_04_innerScala_kbmMid = v;
    this->mtof_tilde_04_innerScala_updateRefFreq();
}

void mtof_tilde_04_innerScala_ref(int v) {
    this->mtof_tilde_04_innerScala_kbmRefNum = v;
    this->mtof_tilde_04_innerScala_updateRefFreq();
}

void mtof_tilde_04_innerScala_base(number v) {
    this->mtof_tilde_04_innerScala_kbmRefFreq = v;
    this->mtof_tilde_04_innerScala_updateRefFreq();
}

void mtof_tilde_04_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_tilde_04_innerScala_updateScale(sclValid);
}

void mtof_tilde_04_innerScala_update(list scale, list map) {
    if (scale->length > 0) {
        this->mtof_tilde_04_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_tilde_04_innerScala_updateMap(map);
    }
}

number mtof_tilde_04_innerScala_mtof(number note) {
    if ((bool)(this->mtof_tilde_04_innerScala_lastValid) && this->mtof_tilde_04_innerScala_lastNote == note) {
        return this->mtof_tilde_04_innerScala_lastFreq;
    }

    array<int, 2> degoct = this->mtof_tilde_04_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_tilde_04_innerScala_applySCL(degoct[0], fract(note), this->mtof_tilde_04_innerScala_refFreq);
    }

    this->mtof_tilde_04_innerScala_updateLast(note, out);
    return out;
}

number mtof_tilde_04_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_tilde_04_innerScala_lastValid) && this->mtof_tilde_04_innerScala_lastFreq == hz) {
        return this->mtof_tilde_04_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_tilde_04_innerScala_hztodeg(hz);
    int degree = (int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_tilde_04_innerScala_kbmSize == 0) {
        out = this->mtof_tilde_04_innerScala_kbmMid + degree;
    } else {
        array<int, 2> octdeg = this->mtof_tilde_04_innerScala_octdegree(degree, this->mtof_tilde_04_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_tilde_04_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_tilde_04_innerScala_kbmValid[(Index)(i + this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_tilde_04_innerScala_kbmSize + entry + this->mtof_tilde_04_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_tilde_04_innerScala_updateLast(out, hz);
    return this->mtof_tilde_04_innerScala_lastNote;
}

int mtof_tilde_04_innerScala_updateScale(list scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_tilde_04_innerScala_lastValid = false;
        this->mtof_tilde_04_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_tilde_04_innerScala_sclExpMul->push(last);
        }

        this->mtof_tilde_04_innerScala_sclOctaveMul = last;
        this->mtof_tilde_04_innerScala_sclEntryCount = (int)(this->mtof_tilde_04_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_tilde_04_innerScala_kbmMid = (int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_tilde_04_innerScala_kbmRefNum = (int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_tilde_04_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_tilde_04_innerScala_kbmSize = (int)(0);
        }

        this->mtof_tilde_04_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

int mtof_tilde_04_innerScala_updateMap(list kbm) {
    if (kbm->length == 1 && kbm[0] == 0.0) {
        kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (kbm->length >= 6 && kbm[0] >= 0.0) {
        this->mtof_tilde_04_innerScala_lastValid = false;
        Index size = (Index)(kbm[0]);
        int octave = 12;

        if (kbm->length > 6) {
            octave = (int)(kbm[6]);
        }

        if (size > 0 && kbm->length < this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_tilde_04_innerScala_kbmSize = (int)(size);
        this->mtof_tilde_04_innerScala_kbmMin = (int)(kbm[1]);
        this->mtof_tilde_04_innerScala_kbmMax = (int)(kbm[2]);
        this->mtof_tilde_04_innerScala_kbmMid = (int)(kbm[3]);
        this->mtof_tilde_04_innerScala_kbmRefNum = (int)(kbm[4]);
        this->mtof_tilde_04_innerScala_kbmRefFreq = kbm[5];
        this->mtof_tilde_04_innerScala_kbmOctaveDegree = octave;
        this->mtof_tilde_04_innerScala_kbmValid = kbm;
        this->mtof_tilde_04_innerScala_kbmMapSize = (kbm->length - this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET > kbm->length ? kbm->length : (kbm->length - this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET < 0 ? 0 : kbm->length - this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET));
        this->mtof_tilde_04_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_tilde_04_innerScala_updateLast(number note, number freq) {
    this->mtof_tilde_04_innerScala_lastValid = true;
    this->mtof_tilde_04_innerScala_lastNote = note;
    this->mtof_tilde_04_innerScala_lastFreq = freq;
}

array<number, 2> mtof_tilde_04_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_tilde_04_innerScala_refFreq) / this->mtof_tilde_04_innerScala_sclOctaveMul
    );

    int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_tilde_04_innerScala_sclEntryCount; i++) {
        number c = this->mtof_tilde_04_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_tilde_04_innerScala_refFreq);
        n = this->mtof_tilde_04_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_tilde_04_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_tilde_04_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_tilde_04_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_tilde_04_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_tilde_04_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<int, 2> mtof_tilde_04_innerScala_octdegree(int degree, int count) {
    int octave = 0;
    int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<int, 2> mtof_tilde_04_innerScala_applyKBM(number note) {
    if ((this->mtof_tilde_04_innerScala_kbmMin == this->mtof_tilde_04_innerScala_kbmMax && this->mtof_tilde_04_innerScala_kbmMax == 0) || (note >= this->mtof_tilde_04_innerScala_kbmMin && note <= this->mtof_tilde_04_innerScala_kbmMax)) {
        int degree = (int)(rnbo_floor(note - this->mtof_tilde_04_innerScala_kbmMid));

        if (this->mtof_tilde_04_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<int, 2> octdeg = this->mtof_tilde_04_innerScala_octdegree(degree, this->mtof_tilde_04_innerScala_kbmSize);
        int octave = (int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_tilde_04_innerScala_kbmMapSize > index) {
            degree = (int)(this->mtof_tilde_04_innerScala_kbmValid[(Index)(this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_tilde_04_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_tilde_04_innerScala_applySCL(int degree, number frac, number refFreq) {
    array<int, 2> octdeg = this->mtof_tilde_04_innerScala_octdegree(degree, this->mtof_tilde_04_innerScala_sclEntryCount);
    return this->mtof_tilde_04_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_tilde_04_innerScala_applySCLOctIndex(number octave, int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_tilde_04_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_tilde_04_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_tilde_04_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_tilde_04_innerScala_sclOctaveMul);
}

void mtof_tilde_04_innerScala_updateRefFreq() {
    this->mtof_tilde_04_innerScala_lastValid = false;
    int refOffset = (int)(this->mtof_tilde_04_innerScala_kbmRefNum - this->mtof_tilde_04_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_tilde_04_innerScala_refFreq = this->mtof_tilde_04_innerScala_kbmRefFreq;
    } else {
        int base = (int)(this->mtof_tilde_04_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_tilde_04_innerScala_sclEntryCount;
        }

        array<int, 2> octdeg = this->mtof_tilde_04_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        int index = (int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_tilde_04_innerScala_kbmSize) {
            if (index < this->mtof_tilde_04_innerScala_kbmMapSize) {
                index = (int)(this->mtof_tilde_04_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_tilde_04_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > this->mtof_tilde_04_innerScala_sclExpMul->length)
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_tilde_04_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_tilde_04_innerScala_refFreq = this->mtof_tilde_04_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_tilde_04_innerScala_sclOctaveMul);
        }
    }
}

void mtof_tilde_04_innerScala_reset() {
    this->mtof_tilde_04_innerScala_lastValid = false;
    this->mtof_tilde_04_innerScala_lastNote = 0;
    this->mtof_tilde_04_innerScala_lastFreq = 0;
    this->mtof_tilde_04_innerScala_sclEntryCount = 0;
    this->mtof_tilde_04_innerScala_sclOctaveMul = 1;
    this->mtof_tilde_04_innerScala_sclExpMul = {};
    this->mtof_tilde_04_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_tilde_04_innerScala_kbmMid = 60;
    this->mtof_tilde_04_innerScala_kbmRefNum = 69;
    this->mtof_tilde_04_innerScala_kbmRefFreq = 440;
    this->mtof_tilde_04_innerScala_kbmSize = 0;
    this->mtof_tilde_04_innerScala_kbmMin = 0;
    this->mtof_tilde_04_innerScala_kbmMax = 0;
    this->mtof_tilde_04_innerScala_kbmOctaveDegree = 12;
    this->mtof_tilde_04_innerScala_kbmMapSize = 0;
    this->mtof_tilde_04_innerScala_refFreq = 261.63;
}

void mtof_tilde_04_init() {
    this->mtof_tilde_04_innerScala_update(this->mtof_tilde_04_scale, this->mtof_tilde_04_map);
}

number line_11_valueAtTime(MillisecondTime time) {
    return this->line_11_startValue + this->line_11_slope * (time - this->line_11_startTime);
}

void line_11_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_11_grain;
    number nextValue;

    if (nextTime - this->line_11_startTime >= this->line_11__time || this->line_11_grain == 0) {
        nextTime = this->line_11_startTime + this->line_11__time;
        nextValue = this->line_11_currentTarget;
    } else {
        nextValue = this->line_11_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 1220262738, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_11_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_11_startValue = this->line_11_valueAtTime(currentTime);
    this->line_11_startTime = currentTime;
    this->line_11_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 1220262738, false);;
    number rise = target - this->line_11_startValue;
    this->line_11_slope = rise / time;
    this->line_11_scheduleNext();
}

bool line_11_isFinished(number value) {
    return value == this->line_11_currentTarget && this->currenttime() - this->line_11_startTime >= this->line_11__time;
}

void line_11_startPendingRamp() {
    if (this->line_11_pendingRamps->length < 2) {
        this->line_11_pendingRamps->length = 0;
        this->line_11__time = 0;
        this->line_11_time_set(0);
        this->line_11_end_bang();
        return;
    }

    if (this->line_11_pendingRamps->length > 1) {
        number target = this->line_11_pendingRamps->shift();
        this->line_11__time = this->line_11_pendingRamps->shift();
        this->line_11__time = (this->line_11__time < 0 ? 0 : this->line_11__time);
        this->line_11_startRamp(target, this->line_11__time);
    }
}

void message_13_init() {
    this->message_13_set_set({122, 2000});
}

void message_14_init() {
    this->message_14_set_set({2000});
}

void slider_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->slider_08_value;
}

void slider_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->slider_08_value_set(preset["value"]);
}

number groove_04_getSample(
    Index channel,
    SampleIndex index,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    if (offset > 0) {
        index += offset;

        if (index >= bufferLength)
            index -= bufferLength;
    }

    return this->groove_04_buffer->getSample(channel, index);
}

number groove_04_interpolatedSample(
    Index channel,
    number index,
    SampleIndex end,
    SampleIndex length,
    SampleIndex offset,
    SampleIndex bufferLength
) {
    SampleIndex index1 = (SampleIndex)(trunc(index));
    number i_x = index - index1;
    number i_1px = 1. + i_x;
    number i_1mx = 1. - i_x;
    number i_2mx = 2. - i_x;
    number i_a = i_1mx * i_2mx;
    number i_b = i_1px * i_x;
    number i_p1 = -.1666667 * i_a * i_x;
    number i_p2 = .5 * i_1px * i_a;
    number i_p3 = .5 * i_b * i_2mx;
    number i_p4 = -.1666667 * i_b * i_1mx;
    SampleIndex index2 = (SampleIndex)(index1 + 1);

    if (index2 >= end)
        index2 -= length;

    SampleIndex index3 = (SampleIndex)(index1 + 2);

    if (index3 >= end)
        index3 -= length;

    SampleIndex index4 = (SampleIndex)(index1 + 3);

    if (index4 >= end)
        index4 -= length;

    return this->groove_04_getSample(channel, index1, offset, bufferLength) * i_p1 + this->groove_04_getSample(channel, index2, offset, bufferLength) * i_p2 + this->groove_04_getSample(channel, index3, offset, bufferLength) * i_p3 + this->groove_04_getSample(channel, index4, offset, bufferLength) * i_p4;
}

number groove_04_crossfadedSample(
    SampleValue out,
    number readIndex,
    Index channel,
    number rate,
    number loopMin,
    number loopMax,
    number loopLength,
    number offset,
    number bufferLength
) {
    auto crossFadeStart1 = this->maximum(loopMin - this->groove_04_crossfadeInSamples, 0);
    auto crossFadeEnd1 = this->minimum(crossFadeStart1 + this->groove_04_crossfadeInSamples, bufferLength);
    number crossFadeStart2 = crossFadeStart1 + loopLength;
    auto crossFadeEnd2 = this->minimum(crossFadeEnd1 + loopLength, bufferLength);
    number crossFadeLength = crossFadeEnd2 - crossFadeStart2;

    if (crossFadeLength > 0) {
        crossFadeEnd1 = crossFadeStart1 + crossFadeLength;
        number diff = -1;
        number addFactor = 0;

        if (readIndex >= crossFadeStart2) {
            diff = readIndex - crossFadeStart2;
            addFactor = -1;
        } else if (readIndex < crossFadeEnd1) {
            diff = crossFadeEnd1 - readIndex + loopMax - crossFadeStart2;
            addFactor = 1;
        }

        if (diff >= 0) {
            number out2ReadIndex = readIndex + loopLength * addFactor;
            number out2 = (rate == 1 ? this->groove_04_getSample(channel, trunc(out2ReadIndex), offset, bufferLength) : this->groove_04_interpolatedSample(channel, out2ReadIndex, loopMax, loopLength, offset, bufferLength));
            number out2Factor = diff / crossFadeLength;
            number out1Factor = 1 - out2Factor;
            return out * out1Factor + out2 * out2Factor;
        }
    }

    return out;
}

void groove_04_dspsetup(bool force) {
    if ((bool)(this->groove_04_setupDone) && (bool)(!(bool)(force)))
        return;

    this->groove_04_crossfadeInSamples = this->mstosamps(this->groove_04_crossfade);
    this->groove_04_setupDone = true;
}

void toggle_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_05_value_number;
}

void toggle_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_05_value_number_set(preset["value"]);
}

number pan_tilde_05_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_05_func_reset() {}

number gaintilde_05_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_05_ramp_d_prev);
    this->gaintilde_05_ramp_d_prev = x;
    return temp;
}

void gaintilde_05_ramp_d_dspsetup() {
    this->gaintilde_05_ramp_d_reset();
}

void gaintilde_05_ramp_d_reset() {
    this->gaintilde_05_ramp_d_prev = 0;
}

number gaintilde_05_ramp_next(number x, number up, number down) {
    if (this->gaintilde_05_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_05_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_05_ramp_index = _up;
            this->gaintilde_05_ramp_increment = (x - this->gaintilde_05_ramp_prev) / _up;
        } else if (x < this->gaintilde_05_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_05_ramp_index = _down;
            this->gaintilde_05_ramp_increment = (x - this->gaintilde_05_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_05_ramp_index > 0) {
        this->gaintilde_05_ramp_prev += this->gaintilde_05_ramp_increment;
        this->gaintilde_05_ramp_index -= 1;
    } else {
        this->gaintilde_05_ramp_prev = x;
    }

    return this->gaintilde_05_ramp_prev;
}

void gaintilde_05_ramp_reset() {
    this->gaintilde_05_ramp_prev = 0;
    this->gaintilde_05_ramp_index = 0;
    this->gaintilde_05_ramp_increment = 0;
    this->gaintilde_05_ramp_d_reset();
}

void gaintilde_05_init() {
    this->gaintilde_05_loginc = rnbo_log(1.072);
    this->gaintilde_05_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_05_loginc));
}

void gaintilde_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_05_value;
}

void gaintilde_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_05_value_set(preset["value"]);
}

void gaintilde_05_dspsetup(bool force) {
    if ((bool)(this->gaintilde_05_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_05_setupDone = true;
    this->gaintilde_05_ramp_d_dspsetup();
}

void message_15_init() {
    this->message_15_set_set({0.5, 3000, 0.4, 8000});
}

number line_12_valueAtTime(MillisecondTime time) {
    return this->line_12_startValue + this->line_12_slope * (time - this->line_12_startTime);
}

void line_12_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_12_grain;
    number nextValue;

    if (nextTime - this->line_12_startTime >= this->line_12__time || this->line_12_grain == 0) {
        nextTime = this->line_12_startTime + this->line_12__time;
        nextValue = this->line_12_currentTarget;
    } else {
        nextValue = this->line_12_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -1916497906, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_12_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_12_startValue = this->line_12_valueAtTime(currentTime);
    this->line_12_startTime = currentTime;
    this->line_12_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -1916497906, false);;
    number rise = target - this->line_12_startValue;
    this->line_12_slope = rise / time;
    this->line_12_scheduleNext();
}

bool line_12_isFinished(number value) {
    return value == this->line_12_currentTarget && this->currenttime() - this->line_12_startTime >= this->line_12__time;
}

void line_12_startPendingRamp() {
    if (this->line_12_pendingRamps->length < 2) {
        this->line_12_pendingRamps->length = 0;
        this->line_12__time = 0;
        this->line_12_time_set(0);
        this->line_12_end_bang();
        return;
    }

    if (this->line_12_pendingRamps->length > 1) {
        number target = this->line_12_pendingRamps->shift();
        this->line_12__time = this->line_12_pendingRamps->shift();
        this->line_12__time = (this->line_12__time < 0 ? 0 : this->line_12__time);
        this->line_12_startRamp(target, this->line_12__time);
    }
}

void numberobj_10_init() {
    this->numberobj_10_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-37"), 1, this->_currentTime);
}

void numberobj_10_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_10_value;
}

void numberobj_10_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_10_value_set(preset["value"]);
}

void message_16_init() {
    this->message_16_set_set({100});
}

number pan_tilde_06_func_next(number pos, int channel) {
    {
        {
            number nchan_1 = 2 - 1;

            {
                pos = pos * nchan_1;
            }

            {
                {
                    {
                        if (pos <= -1 || pos >= 2)
                            return 0;
                    }
                }
            }

            pos = pos - channel;

            if (pos > -1 && pos < 1) {
                {
                    {
                        return this->safesqrt(1.0 - rnbo_abs(pos));
                    }
                }
            } else {
                return 0;
            }
        }
    }
}

void pan_tilde_06_func_reset() {}

number gaintilde_06_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_06_ramp_d_prev);
    this->gaintilde_06_ramp_d_prev = x;
    return temp;
}

void gaintilde_06_ramp_d_dspsetup() {
    this->gaintilde_06_ramp_d_reset();
}

void gaintilde_06_ramp_d_reset() {
    this->gaintilde_06_ramp_d_prev = 0;
}

number gaintilde_06_ramp_next(number x, number up, number down) {
    if (this->gaintilde_06_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_06_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_06_ramp_index = _up;
            this->gaintilde_06_ramp_increment = (x - this->gaintilde_06_ramp_prev) / _up;
        } else if (x < this->gaintilde_06_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_06_ramp_index = _down;
            this->gaintilde_06_ramp_increment = (x - this->gaintilde_06_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_06_ramp_index > 0) {
        this->gaintilde_06_ramp_prev += this->gaintilde_06_ramp_increment;
        this->gaintilde_06_ramp_index -= 1;
    } else {
        this->gaintilde_06_ramp_prev = x;
    }

    return this->gaintilde_06_ramp_prev;
}

void gaintilde_06_ramp_reset() {
    this->gaintilde_06_ramp_prev = 0;
    this->gaintilde_06_ramp_index = 0;
    this->gaintilde_06_ramp_increment = 0;
    this->gaintilde_06_ramp_d_reset();
}

void gaintilde_06_init() {
    this->gaintilde_06_loginc = rnbo_log(1.072);
    this->gaintilde_06_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_06_loginc));
}

void gaintilde_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_06_value;
}

void gaintilde_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_06_value_set(preset["value"]);
}

void gaintilde_06_dspsetup(bool force) {
    if ((bool)(this->gaintilde_06_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_06_setupDone = true;
    this->gaintilde_06_ramp_d_dspsetup();
}

void message_17_init() {
    this->message_17_set_set({0.5, 3000, 1.2, 8000});
}

number line_13_valueAtTime(MillisecondTime time) {
    return this->line_13_startValue + this->line_13_slope * (time - this->line_13_startTime);
}

void line_13_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_13_grain;
    number nextValue;

    if (nextTime - this->line_13_startTime >= this->line_13__time || this->line_13_grain == 0) {
        nextTime = this->line_13_startTime + this->line_13__time;
        nextValue = this->line_13_currentTarget;
    } else {
        nextValue = this->line_13_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 2006462159, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_13_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_13_startValue = this->line_13_valueAtTime(currentTime);
    this->line_13_startTime = currentTime;
    this->line_13_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 2006462159, false);;
    number rise = target - this->line_13_startValue;
    this->line_13_slope = rise / time;
    this->line_13_scheduleNext();
}

bool line_13_isFinished(number value) {
    return value == this->line_13_currentTarget && this->currenttime() - this->line_13_startTime >= this->line_13__time;
}

void line_13_startPendingRamp() {
    if (this->line_13_pendingRamps->length < 2) {
        this->line_13_pendingRamps->length = 0;
        this->line_13__time = 0;
        this->line_13_time_set(0);
        this->line_13_end_bang();
        return;
    }

    if (this->line_13_pendingRamps->length > 1) {
        number target = this->line_13_pendingRamps->shift();
        this->line_13__time = this->line_13_pendingRamps->shift();
        this->line_13__time = (this->line_13__time < 0 ? 0 : this->line_13__time);
        this->line_13_startRamp(target, this->line_13__time);
    }
}

number gaintilde_07_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_07_ramp_d_prev);
    this->gaintilde_07_ramp_d_prev = x;
    return temp;
}

void gaintilde_07_ramp_d_dspsetup() {
    this->gaintilde_07_ramp_d_reset();
}

void gaintilde_07_ramp_d_reset() {
    this->gaintilde_07_ramp_d_prev = 0;
}

number gaintilde_07_ramp_next(number x, number up, number down) {
    if (this->gaintilde_07_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_07_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_07_ramp_index = _up;
            this->gaintilde_07_ramp_increment = (x - this->gaintilde_07_ramp_prev) / _up;
        } else if (x < this->gaintilde_07_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_07_ramp_index = _down;
            this->gaintilde_07_ramp_increment = (x - this->gaintilde_07_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_07_ramp_index > 0) {
        this->gaintilde_07_ramp_prev += this->gaintilde_07_ramp_increment;
        this->gaintilde_07_ramp_index -= 1;
    } else {
        this->gaintilde_07_ramp_prev = x;
    }

    return this->gaintilde_07_ramp_prev;
}

void gaintilde_07_ramp_reset() {
    this->gaintilde_07_ramp_prev = 0;
    this->gaintilde_07_ramp_index = 0;
    this->gaintilde_07_ramp_increment = 0;
    this->gaintilde_07_ramp_d_reset();
}

void gaintilde_07_init() {
    this->gaintilde_07_loginc = rnbo_log(1.072);
    this->gaintilde_07_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_07_loginc));
}

void gaintilde_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_07_value;
}

void gaintilde_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_07_value_set(preset["value"]);
}

void gaintilde_07_dspsetup(bool force) {
    if ((bool)(this->gaintilde_07_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_07_setupDone = true;
    this->gaintilde_07_ramp_d_dspsetup();
}

void random_06_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_06_nz_state
    );
}

void random_06_nz_init() {
    this->random_06_nz_reset();
}

void random_06_nz_seed(number v) {
    xoshiro_reset(v, this->random_06_nz_state);
}

number random_06_nz_next() {
    return xoshiro_next(this->random_06_nz_state);
}

void numberobj_11_init() {
    this->numberobj_11_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-82"), 1, this->_currentTime);
}

void numberobj_11_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_11_value;
}

void numberobj_11_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_11_value_set(preset["value"]);
}

void toggle_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_06_value_number;
}

void toggle_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_06_value_number_set(preset["value"]);
}

void metro_02_onTransportChanged(number ) {}

void metro_02_onBeatTimeChanged(number ) {}

void counter_02_inc() {
    this->counter_02_count = ((bool)(this->counter_02_phase) ? this->counter_02_count + 1 : this->counter_02_count - 1);
}

void random_07_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_07_nz_state
    );
}

void random_07_nz_init() {
    this->random_07_nz_reset();
}

void random_07_nz_seed(number v) {
    xoshiro_reset(v, this->random_07_nz_state);
}

number random_07_nz_next() {
    return xoshiro_next(this->random_07_nz_state);
}

void random_08_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_08_nz_state
    );
}

void random_08_nz_init() {
    this->random_08_nz_reset();
}

void random_08_nz_seed(number v) {
    xoshiro_reset(v, this->random_08_nz_state);
}

number random_08_nz_next() {
    return xoshiro_next(this->random_08_nz_state);
}

number line_14_valueAtTime(MillisecondTime time) {
    return this->line_14_startValue + this->line_14_slope * (time - this->line_14_startTime);
}

void line_14_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_14_grain;
    number nextValue;

    if (nextTime - this->line_14_startTime >= this->line_14__time || this->line_14_grain == 0) {
        nextTime = this->line_14_startTime + this->line_14__time;
        nextValue = this->line_14_currentTarget;
    } else {
        nextValue = this->line_14_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 1964277200, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_14_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_14_startValue = this->line_14_valueAtTime(currentTime);
    this->line_14_startTime = currentTime;
    this->line_14_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 1964277200, false);;
    number rise = target - this->line_14_startValue;
    this->line_14_slope = rise / time;
    this->line_14_scheduleNext();
}

bool line_14_isFinished(number value) {
    return value == this->line_14_currentTarget && this->currenttime() - this->line_14_startTime >= this->line_14__time;
}

void line_14_startPendingRamp() {
    if (this->line_14_pendingRamps->length < 2) {
        this->line_14_pendingRamps->length = 0;
        this->line_14__time = 0;
        this->line_14_time_set(0);
        this->line_14_end_bang();
        return;
    }

    if (this->line_14_pendingRamps->length > 1) {
        number target = this->line_14_pendingRamps->shift();
        this->line_14__time = this->line_14_pendingRamps->shift();
        this->line_14__time = (this->line_14__time < 0 ? 0 : this->line_14__time);
        this->line_14_startRamp(target, this->line_14__time);
    }
}

number gaintilde_08_ramp_d_next(number x) {
    number temp = (number)(x - this->gaintilde_08_ramp_d_prev);
    this->gaintilde_08_ramp_d_prev = x;
    return temp;
}

void gaintilde_08_ramp_d_dspsetup() {
    this->gaintilde_08_ramp_d_reset();
}

void gaintilde_08_ramp_d_reset() {
    this->gaintilde_08_ramp_d_prev = 0;
}

number gaintilde_08_ramp_next(number x, number up, number down) {
    if (this->gaintilde_08_ramp_d_next(x) != 0.) {
        if (x > this->gaintilde_08_ramp_prev) {
            number _up = up;

            if (_up < 1)
                _up = 1;

            this->gaintilde_08_ramp_index = _up;
            this->gaintilde_08_ramp_increment = (x - this->gaintilde_08_ramp_prev) / _up;
        } else if (x < this->gaintilde_08_ramp_prev) {
            number _down = down;

            if (_down < 1)
                _down = 1;

            this->gaintilde_08_ramp_index = _down;
            this->gaintilde_08_ramp_increment = (x - this->gaintilde_08_ramp_prev) / _down;
        }
    }

    if (this->gaintilde_08_ramp_index > 0) {
        this->gaintilde_08_ramp_prev += this->gaintilde_08_ramp_increment;
        this->gaintilde_08_ramp_index -= 1;
    } else {
        this->gaintilde_08_ramp_prev = x;
    }

    return this->gaintilde_08_ramp_prev;
}

void gaintilde_08_ramp_reset() {
    this->gaintilde_08_ramp_prev = 0;
    this->gaintilde_08_ramp_index = 0;
    this->gaintilde_08_ramp_increment = 0;
    this->gaintilde_08_ramp_d_reset();
}

void gaintilde_08_init() {
    this->gaintilde_08_loginc = rnbo_log(1.072);
    this->gaintilde_08_zval = 7.943 * rnbo_exp(-((158 - 1) * this->gaintilde_08_loginc));
}

void gaintilde_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->gaintilde_08_value;
}

void gaintilde_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->gaintilde_08_value_set(preset["value"]);
}

void gaintilde_08_dspsetup(bool force) {
    if ((bool)(this->gaintilde_08_setupDone) && (bool)(!(bool)(force)))
        return;

    this->gaintilde_08_setupDone = true;
    this->gaintilde_08_ramp_d_dspsetup();
}

void message_18_init() {
    this->message_18_set_set({0, 10000});
}

void param_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_03_value;
}

void param_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_03_value_set(preset["value"]);
}

void numberobj_12_init() {
    this->numberobj_12_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-100"), 1, this->_currentTime);
}

void numberobj_12_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_12_value;
}

void numberobj_12_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_12_value_set(preset["value"]);
}

void numberobj_13_init() {
    this->numberobj_13_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-262"), 1, this->_currentTime);
}

void numberobj_13_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_13_value;
}

void numberobj_13_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_13_value_set(preset["value"]);
}

void param_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_04_value;
}

void param_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_04_value_set(preset["value"]);
}

void numberobj_14_init() {
    this->numberobj_14_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-85"), 1, this->_currentTime);
}

void numberobj_14_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_14_value;
}

void numberobj_14_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_14_value_set(preset["value"]);
}

void message_19_init() {
    this->message_19_set_set({2000});
}

void numberobj_15_init() {
    this->numberobj_15_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-94"), 1, this->_currentTime);
}

void numberobj_15_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_15_value;
}

void numberobj_15_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_15_value_set(preset["value"]);
}

number line_15_valueAtTime(MillisecondTime time) {
    return this->line_15_startValue + this->line_15_slope * (time - this->line_15_startTime);
}

void line_15_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_15_grain;
    number nextValue;

    if (nextTime - this->line_15_startTime >= this->line_15__time || this->line_15_grain == 0) {
        nextTime = this->line_15_startTime + this->line_15__time;
        nextValue = this->line_15_currentTarget;
    } else {
        nextValue = this->line_15_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 146426004, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_15_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_15_startValue = this->line_15_valueAtTime(currentTime);
    this->line_15_startTime = currentTime;
    this->line_15_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 146426004, false);;
    number rise = target - this->line_15_startValue;
    this->line_15_slope = rise / time;
    this->line_15_scheduleNext();
}

bool line_15_isFinished(number value) {
    return value == this->line_15_currentTarget && this->currenttime() - this->line_15_startTime >= this->line_15__time;
}

void line_15_startPendingRamp() {
    if (this->line_15_pendingRamps->length < 2) {
        this->line_15_pendingRamps->length = 0;
        this->line_15__time = 0;
        this->line_15_time_set(0);
        this->line_15_end_bang();
        return;
    }

    if (this->line_15_pendingRamps->length > 1) {
        number target = this->line_15_pendingRamps->shift();
        this->line_15__time = this->line_15_pendingRamps->shift();
        this->line_15__time = (this->line_15__time < 0 ? 0 : this->line_15__time);
        this->line_15_startRamp(target, this->line_15__time);
    }
}

void random_09_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_09_nz_state
    );
}

void random_09_nz_init() {
    this->random_09_nz_reset();
}

void random_09_nz_seed(number v) {
    xoshiro_reset(v, this->random_09_nz_state);
}

number random_09_nz_next() {
    return xoshiro_next(this->random_09_nz_state);
}

void numberobj_16_init() {
    this->numberobj_16_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-236"), 1, this->_currentTime);
}

void numberobj_16_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_16_value;
}

void numberobj_16_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_16_value_set(preset["value"]);
}

void message_20_init() {
    this->message_20_set_set({100, 100, 128, 5000});
}

void numberobj_17_init() {
    this->numberobj_17_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-121"), 1, this->_currentTime);
}

void numberobj_17_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_17_value;
}

void numberobj_17_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_17_value_set(preset["value"]);
}

number line_16_valueAtTime(MillisecondTime time) {
    return this->line_16_startValue + this->line_16_slope * (time - this->line_16_startTime);
}

void line_16_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_16_grain;
    number nextValue;

    if (nextTime - this->line_16_startTime >= this->line_16__time || this->line_16_grain == 0) {
        nextTime = this->line_16_startTime + this->line_16__time;
        nextValue = this->line_16_currentTarget;
    } else {
        nextValue = this->line_16_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, 848255507, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_16_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_16_startValue = this->line_16_valueAtTime(currentTime);
    this->line_16_startTime = currentTime;
    this->line_16_currentTarget = target;
    this->getEngine()->flushClockEvents(this, 848255507, false);;
    number rise = target - this->line_16_startValue;
    this->line_16_slope = rise / time;
    this->line_16_scheduleNext();
}

bool line_16_isFinished(number value) {
    return value == this->line_16_currentTarget && this->currenttime() - this->line_16_startTime >= this->line_16__time;
}

void line_16_startPendingRamp() {
    if (this->line_16_pendingRamps->length < 2) {
        this->line_16_pendingRamps->length = 0;
        this->line_16__time = 0;
        this->line_16_time_set(0);
        this->line_16_end_bang();
        return;
    }

    if (this->line_16_pendingRamps->length > 1) {
        number target = this->line_16_pendingRamps->shift();
        this->line_16__time = this->line_16_pendingRamps->shift();
        this->line_16__time = (this->line_16__time < 0 ? 0 : this->line_16__time);
        this->line_16_startRamp(target, this->line_16__time);
    }
}

void message_21_init() {
    this->message_21_set_set({3000});
}

void counter_03_inc() {
    this->counter_03_count = ((bool)(this->counter_03_phase) ? this->counter_03_count + 1 : this->counter_03_count - 1);
}

void numberobj_18_init() {
    this->numberobj_18_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-44"), 1, this->_currentTime);
}

void numberobj_18_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_18_value;
}

void numberobj_18_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_18_value_set(preset["value"]);
}

void random_10_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_10_nz_state
    );
}

void random_10_nz_init() {
    this->random_10_nz_reset();
}

void random_10_nz_seed(number v) {
    xoshiro_reset(v, this->random_10_nz_state);
}

number random_10_nz_next() {
    return xoshiro_next(this->random_10_nz_state);
}

void numberobj_19_init() {
    this->numberobj_19_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-151"), 1, this->_currentTime);
}

void numberobj_19_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_19_value;
}

void numberobj_19_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_19_value_set(preset["value"]);
}

void toggle_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_07_value_number;
}

void toggle_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_07_value_number_set(preset["value"]);
}

void metro_03_onTransportChanged(number ) {}

void metro_03_onBeatTimeChanged(number ) {}

void counter_04_inc() {
    this->counter_04_count = ((bool)(this->counter_04_phase) ? this->counter_04_count + 1 : this->counter_04_count - 1);
}

void random_11_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_11_nz_state
    );
}

void random_11_nz_init() {
    this->random_11_nz_reset();
}

void random_11_nz_seed(number v) {
    xoshiro_reset(v, this->random_11_nz_state);
}

number random_11_nz_next() {
    return xoshiro_next(this->random_11_nz_state);
}

void numberobj_20_init() {
    this->numberobj_20_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-142"), 1, this->_currentTime);
}

void numberobj_20_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_20_value;
}

void numberobj_20_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_20_value_set(preset["value"]);
}

void numberobj_21_init() {
    this->numberobj_21_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-149"), 1, this->_currentTime);
}

void numberobj_21_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_21_value;
}

void numberobj_21_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_21_value_set(preset["value"]);
}

void numberobj_22_init() {
    this->numberobj_22_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-144"), 1, this->_currentTime);
}

void numberobj_22_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_22_value;
}

void numberobj_22_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_22_value_set(preset["value"]);
}

number line_17_valueAtTime(MillisecondTime time) {
    return this->line_17_startValue + this->line_17_slope * (time - this->line_17_startTime);
}

void line_17_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_17_grain;
    number nextValue;

    if (nextTime - this->line_17_startTime >= this->line_17__time || this->line_17_grain == 0) {
        nextTime = this->line_17_startTime + this->line_17__time;
        nextValue = this->line_17_currentTarget;
    } else {
        nextValue = this->line_17_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -225581227, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_17_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_17_startValue = this->line_17_valueAtTime(currentTime);
    this->line_17_startTime = currentTime;
    this->line_17_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -225581227, false);;
    number rise = target - this->line_17_startValue;
    this->line_17_slope = rise / time;
    this->line_17_scheduleNext();
}

bool line_17_isFinished(number value) {
    return value == this->line_17_currentTarget && this->currenttime() - this->line_17_startTime >= this->line_17__time;
}

void line_17_startPendingRamp() {
    if (this->line_17_pendingRamps->length < 2) {
        this->line_17_pendingRamps->length = 0;
        this->line_17__time = 0;
        this->line_17_time_set(0);
        this->line_17_end_bang();
        return;
    }

    if (this->line_17_pendingRamps->length > 1) {
        number target = this->line_17_pendingRamps->shift();
        this->line_17__time = this->line_17_pendingRamps->shift();
        this->line_17__time = (this->line_17__time < 0 ? 0 : this->line_17__time);
        this->line_17_startRamp(target, this->line_17__time);
    }
}

void random_12_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_12_nz_state
    );
}

void random_12_nz_init() {
    this->random_12_nz_reset();
}

void random_12_nz_seed(number v) {
    xoshiro_reset(v, this->random_12_nz_state);
}

number random_12_nz_next() {
    return xoshiro_next(this->random_12_nz_state);
}

void numberobj_23_init() {
    this->numberobj_23_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-137"), 1, this->_currentTime);
}

void numberobj_23_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_23_value;
}

void numberobj_23_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_23_value_set(preset["value"]);
}

void message_22_init() {
    this->message_22_set_set({3000});
}

void counter_05_inc() {
    this->counter_05_count = ((bool)(this->counter_05_phase) ? this->counter_05_count + 1 : this->counter_05_count - 1);
}

void numberobj_24_init() {
    this->numberobj_24_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-156"), 1, this->_currentTime);
}

void numberobj_24_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_24_value;
}

void numberobj_24_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_24_value_set(preset["value"]);
}

void random_13_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_13_nz_state
    );
}

void random_13_nz_init() {
    this->random_13_nz_reset();
}

void random_13_nz_seed(number v) {
    xoshiro_reset(v, this->random_13_nz_state);
}

number random_13_nz_next() {
    return xoshiro_next(this->random_13_nz_state);
}

void numberobj_25_init() {
    this->numberobj_25_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-175"), 1, this->_currentTime);
}

void numberobj_25_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_25_value;
}

void numberobj_25_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_25_value_set(preset["value"]);
}

void toggle_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_08_value_number;
}

void toggle_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_08_value_number_set(preset["value"]);
}

void metro_04_onTransportChanged(number ) {}

void metro_04_onBeatTimeChanged(number ) {}

void counter_06_inc() {
    this->counter_06_count = ((bool)(this->counter_06_phase) ? this->counter_06_count + 1 : this->counter_06_count - 1);
}

void random_14_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_14_nz_state
    );
}

void random_14_nz_init() {
    this->random_14_nz_reset();
}

void random_14_nz_seed(number v) {
    xoshiro_reset(v, this->random_14_nz_state);
}

number random_14_nz_next() {
    return xoshiro_next(this->random_14_nz_state);
}

void numberobj_26_init() {
    this->numberobj_26_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-166"), 1, this->_currentTime);
}

void numberobj_26_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_26_value;
}

void numberobj_26_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_26_value_set(preset["value"]);
}

void numberobj_27_init() {
    this->numberobj_27_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-173"), 1, this->_currentTime);
}

void numberobj_27_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_27_value;
}

void numberobj_27_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_27_value_set(preset["value"]);
}

void numberobj_28_init() {
    this->numberobj_28_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-168"), 1, this->_currentTime);
}

void numberobj_28_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_28_value;
}

void numberobj_28_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_28_value_set(preset["value"]);
}

number line_18_valueAtTime(MillisecondTime time) {
    return this->line_18_startValue + this->line_18_slope * (time - this->line_18_startTime);
}

void line_18_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_18_grain;
    number nextValue;

    if (nextTime - this->line_18_startTime >= this->line_18__time || this->line_18_grain == 0) {
        nextTime = this->line_18_startTime + this->line_18__time;
        nextValue = this->line_18_currentTarget;
    } else {
        nextValue = this->line_18_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -470653941, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_18_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_18_startValue = this->line_18_valueAtTime(currentTime);
    this->line_18_startTime = currentTime;
    this->line_18_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -470653941, false);;
    number rise = target - this->line_18_startValue;
    this->line_18_slope = rise / time;
    this->line_18_scheduleNext();
}

bool line_18_isFinished(number value) {
    return value == this->line_18_currentTarget && this->currenttime() - this->line_18_startTime >= this->line_18__time;
}

void line_18_startPendingRamp() {
    if (this->line_18_pendingRamps->length < 2) {
        this->line_18_pendingRamps->length = 0;
        this->line_18__time = 0;
        this->line_18_time_set(0);
        this->line_18_end_bang();
        return;
    }

    if (this->line_18_pendingRamps->length > 1) {
        number target = this->line_18_pendingRamps->shift();
        this->line_18__time = this->line_18_pendingRamps->shift();
        this->line_18__time = (this->line_18__time < 0 ? 0 : this->line_18__time);
        this->line_18_startRamp(target, this->line_18__time);
    }
}

void random_15_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_15_nz_state
    );
}

void random_15_nz_init() {
    this->random_15_nz_reset();
}

void random_15_nz_seed(number v) {
    xoshiro_reset(v, this->random_15_nz_state);
}

number random_15_nz_next() {
    return xoshiro_next(this->random_15_nz_state);
}

void numberobj_29_init() {
    this->numberobj_29_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-161"), 1, this->_currentTime);
}

void numberobj_29_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_29_value;
}

void numberobj_29_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_29_value_set(preset["value"]);
}

void message_23_init() {
    this->message_23_set_set({3000});
}

void counter_07_inc() {
    this->counter_07_count = ((bool)(this->counter_07_phase) ? this->counter_07_count + 1 : this->counter_07_count - 1);
}

void numberobj_30_init() {
    this->numberobj_30_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-180"), 1, this->_currentTime);
}

void numberobj_30_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_30_value;
}

void numberobj_30_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_30_value_set(preset["value"]);
}

void random_16_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_16_nz_state
    );
}

void random_16_nz_init() {
    this->random_16_nz_reset();
}

void random_16_nz_seed(number v) {
    xoshiro_reset(v, this->random_16_nz_state);
}

number random_16_nz_next() {
    return xoshiro_next(this->random_16_nz_state);
}

void numberobj_31_init() {
    this->numberobj_31_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-199"), 1, this->_currentTime);
}

void numberobj_31_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_31_value;
}

void numberobj_31_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_31_value_set(preset["value"]);
}

void toggle_09_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->toggle_09_value_number;
}

void toggle_09_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->toggle_09_value_number_set(preset["value"]);
}

void metro_05_onTransportChanged(number ) {}

void metro_05_onBeatTimeChanged(number ) {}

void counter_08_inc() {
    this->counter_08_count = ((bool)(this->counter_08_phase) ? this->counter_08_count + 1 : this->counter_08_count - 1);
}

void random_17_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_17_nz_state
    );
}

void random_17_nz_init() {
    this->random_17_nz_reset();
}

void random_17_nz_seed(number v) {
    xoshiro_reset(v, this->random_17_nz_state);
}

number random_17_nz_next() {
    return xoshiro_next(this->random_17_nz_state);
}

void numberobj_32_init() {
    this->numberobj_32_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-190"), 1, this->_currentTime);
}

void numberobj_32_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_32_value;
}

void numberobj_32_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_32_value_set(preset["value"]);
}

void numberobj_33_init() {
    this->numberobj_33_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-197"), 1, this->_currentTime);
}

void numberobj_33_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_33_value;
}

void numberobj_33_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_33_value_set(preset["value"]);
}

void numberobj_34_init() {
    this->numberobj_34_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-192"), 1, this->_currentTime);
}

void numberobj_34_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_34_value;
}

void numberobj_34_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_34_value_set(preset["value"]);
}

number line_19_valueAtTime(MillisecondTime time) {
    return this->line_19_startValue + this->line_19_slope * (time - this->line_19_startTime);
}

void line_19_scheduleNext() {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    number nextTime = currentTime + this->line_19_grain;
    number nextValue;

    if (nextTime - this->line_19_startTime >= this->line_19__time || this->line_19_grain == 0) {
        nextTime = this->line_19_startTime + this->line_19__time;
        nextValue = this->line_19_currentTarget;
    } else {
        nextValue = this->line_19_valueAtTime(nextTime);
    }

    this->getEngine()->scheduleClockEventWithValue(this, -842661172, nextTime - currentTime + this->_currentTime, nextValue);;
}

void line_19_startRamp(number target, MillisecondTime time) {
    MillisecondTime currentTime = (MillisecondTime)(this->currenttime());
    this->line_19_startValue = this->line_19_valueAtTime(currentTime);
    this->line_19_startTime = currentTime;
    this->line_19_currentTarget = target;
    this->getEngine()->flushClockEvents(this, -842661172, false);;
    number rise = target - this->line_19_startValue;
    this->line_19_slope = rise / time;
    this->line_19_scheduleNext();
}

bool line_19_isFinished(number value) {
    return value == this->line_19_currentTarget && this->currenttime() - this->line_19_startTime >= this->line_19__time;
}

void line_19_startPendingRamp() {
    if (this->line_19_pendingRamps->length < 2) {
        this->line_19_pendingRamps->length = 0;
        this->line_19__time = 0;
        this->line_19_time_set(0);
        this->line_19_end_bang();
        return;
    }

    if (this->line_19_pendingRamps->length > 1) {
        number target = this->line_19_pendingRamps->shift();
        this->line_19__time = this->line_19_pendingRamps->shift();
        this->line_19__time = (this->line_19__time < 0 ? 0 : this->line_19__time);
        this->line_19_startRamp(target, this->line_19__time);
    }
}

void random_18_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_18_nz_state
    );
}

void random_18_nz_init() {
    this->random_18_nz_reset();
}

void random_18_nz_seed(number v) {
    xoshiro_reset(v, this->random_18_nz_state);
}

number random_18_nz_next() {
    return xoshiro_next(this->random_18_nz_state);
}

void numberobj_35_init() {
    this->numberobj_35_currentFormat = 6;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-185"), 1, this->_currentTime);
}

void numberobj_35_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_35_value;
}

void numberobj_35_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_35_value_set(preset["value"]);
}

void message_24_init() {
    this->message_24_set_set({3000});
}

void counter_09_inc() {
    this->counter_09_count = ((bool)(this->counter_09_phase) ? this->counter_09_count + 1 : this->counter_09_count - 1);
}

void numberobj_36_init() {
    this->numberobj_36_currentFormat = 0;
    this->getEngine()->sendNumMessage(TAG("setup"), TAG("number_obj-204"), 1, this->_currentTime);
}

void numberobj_36_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_36_value;
}

void numberobj_36_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_36_value_set(preset["value"]);
}

void timevalue_01_sendValue() {
    {
        {
            {
                {
                    this->timevalue_01_out_set(this->tickstoms(240));
                }
            }
        }
    }
}

void timevalue_01_onTempoChanged(number tempo) {
    RNBO_UNUSED(tempo);

    {
        this->timevalue_01_sendValue();
    }
}

void timevalue_01_onSampleRateChanged(number ) {}

void timevalue_01_onTimeSignatureChanged(number , number ) {}

void timevalue_02_sendValue() {
    {
        {
            {
                {
                    this->timevalue_02_out_set(this->tickstoms(480));
                }
            }
        }
    }
}

void timevalue_02_onTempoChanged(number tempo) {
    RNBO_UNUSED(tempo);

    {
        this->timevalue_02_sendValue();
    }
}

void timevalue_02_onSampleRateChanged(number ) {}

void timevalue_02_onTimeSignatureChanged(number , number ) {}

void timevalue_03_sendValue() {
    {
        {
            {
                {
                    this->timevalue_03_out_set(this->tickstoms(480));
                }
            }
        }
    }
}

void timevalue_03_onTempoChanged(number tempo) {
    RNBO_UNUSED(tempo);

    {
        this->timevalue_03_sendValue();
    }
}

void timevalue_03_onSampleRateChanged(number ) {}

void timevalue_03_onTimeSignatureChanged(number , number ) {}

void timevalue_04_sendValue() {
    {
        {
            {
                {
                    this->timevalue_04_out_set(this->tickstoms(480));
                }
            }
        }
    }
}

void timevalue_04_onTempoChanged(number tempo) {
    RNBO_UNUSED(tempo);

    {
        this->timevalue_04_sendValue();
    }
}

void timevalue_04_onSampleRateChanged(number ) {}

void timevalue_04_onTimeSignatureChanged(number , number ) {}

void timevalue_05_sendValue() {
    {
        {
            {
                {
                    this->timevalue_05_out_set(this->tickstoms(480));
                }
            }
        }
    }
}

void timevalue_05_onTempoChanged(number tempo) {
    RNBO_UNUSED(tempo);

    {
        this->timevalue_05_sendValue();
    }
}

void timevalue_05_onSampleRateChanged(number ) {}

void timevalue_05_onTimeSignatureChanged(number , number ) {}

Index globaltransport_getSampleOffset(MillisecondTime time) {
    return this->mstosamps(this->maximum(0, time - this->getEngine()->getCurrentTime()));
}

number globaltransport_getTempoAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < this->vs ? this->globaltransport_tempo[(Index)sampleOffset] : this->globaltransport_lastTempo);
}

number globaltransport_getStateAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < this->vs ? this->globaltransport_state[(Index)sampleOffset] : this->globaltransport_lastState);
}

number globaltransport_getState(MillisecondTime time) {
    return this->globaltransport_getStateAtSample(this->globaltransport_getSampleOffset(time));
}

number globaltransport_getTempo(MillisecondTime time) {
    return this->globaltransport_getTempoAtSample(this->globaltransport_getSampleOffset(time));
}

number globaltransport_getBeatTime(MillisecondTime time) {
    number i = 2;

    while (i < this->globaltransport_beatTimeChanges->length && this->globaltransport_beatTimeChanges[(Index)(i + 1)] <= time) {
        i += 2;
    }

    i -= 2;
    number beatTimeBase = this->globaltransport_beatTimeChanges[(Index)i];

    if (this->globaltransport_getState(time) == 0)
        return beatTimeBase;

    number beatTimeBaseMsTime = this->globaltransport_beatTimeChanges[(Index)(i + 1)];
    number diff = time - beatTimeBaseMsTime;
    number diffInBeats = diff * this->globaltransport_getTempo(time) * 0.008 / (number)480;
    return beatTimeBase + diffInBeats;
}

bool globaltransport_setTempo(MillisecondTime time, number tempo, bool notify) {
    if ((bool)(notify)) {
        this->processTempoEvent(time, tempo);
        this->globaltransport_notify = true;
    } else {
        Index offset = (Index)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getTempoAtSample((SampleIndex)(offset)) != tempo) {
            this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
            this->globaltransport_beatTimeChanges->push(time);
            fillSignal(this->globaltransport_tempo, this->vs, tempo, offset);
            this->globaltransport_lastTempo = tempo;
            this->globaltransport_tempoNeedsReset = true;
            return true;
        }
    }

    return false;
}

bool globaltransport_setState(MillisecondTime time, number state, bool notify) {
    if ((bool)(notify)) {
        this->processTransportEvent(time, TransportState(state));
        this->globaltransport_notify = true;
    } else {
        Index offset = (Index)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getStateAtSample(offset) != state) {
            this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
            this->globaltransport_beatTimeChanges->push(time);
            fillSignal(this->globaltransport_state, this->vs, state, offset);
            this->globaltransport_lastState = TransportState(state);
            this->globaltransport_stateNeedsReset = true;
            return true;
        }
    }

    return false;
}

bool globaltransport_setBeatTime(MillisecondTime time, number beattime, bool notify) {
    if ((bool)(notify)) {
        this->processBeatTimeEvent(time, beattime);
        this->globaltransport_notify = true;
        return false;
    } else {
        bool beatTimeHasChanged = false;
        float oldBeatTime = (float)(this->globaltransport_getBeatTime(time));
        float newBeatTime = (float)(beattime);

        if (oldBeatTime != newBeatTime) {
            beatTimeHasChanged = true;
        }

        this->globaltransport_beatTimeChanges->push(beattime);
        this->globaltransport_beatTimeChanges->push(time);
        return beatTimeHasChanged;
    }
}

number globaltransport_getBeatTimeAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getBeatTime(this->getEngine()->getCurrentTime() + msOffset);
}

array<number, 2> globaltransport_getTimeSignature(MillisecondTime time) {
    number i = 3;

    while (i < this->globaltransport_timeSignatureChanges->length && this->globaltransport_timeSignatureChanges[(Index)(i + 2)] <= time) {
        i += 3;
    }

    i -= 3;

    return {
        this->globaltransport_timeSignatureChanges[(Index)i],
        this->globaltransport_timeSignatureChanges[(Index)(i + 1)]
    };
}

array<number, 2> globaltransport_getTimeSignatureAtSample(SampleIndex sampleOffset) {
    MillisecondTime msOffset = (MillisecondTime)(this->sampstoms(sampleOffset));
    return this->globaltransport_getTimeSignature(this->getEngine()->getCurrentTime() + msOffset);
}

void globaltransport_setBBUBase(
    MillisecondTime time,
    number numerator,
    number denominator,
    number bars,
    number beats,
    number units
) {
    number beatsInQuarterNotes = this->globaltransport_getBeatTime(time);
    bars--;
    beats--;
    number beatsIncCurrenttDenom = beatsInQuarterNotes * (denominator * 0.25);
    number beatLength = (number)4 / denominator;
    number beatLengthInUnits = beatLength * 480;

    while (units > beatLengthInUnits) {
        units -= beatLengthInUnits;
        beats++;
    }

    number targetBeatTime = bars * numerator + beats + units / beatLengthInUnits;
    this->globaltransport_bbuBase = targetBeatTime - beatsIncCurrenttDenom;
}

array<number, 3> globaltransport_getBBU(MillisecondTime time) {
    array<number, 2> currentSig = this->globaltransport_getTimeSignature(time);
    number numerator = currentSig[0];
    number denominator = currentSig[1];
    number beatsInQuarterNotes = this->globaltransport_getBeatTime(time);
    number beatsIncCurrenttDenom = beatsInQuarterNotes * (denominator * 0.25);
    number beatLength = (number)4 / denominator;
    number beatLengthInUnits = beatLength * 480;
    number targetBeatTime = beatsIncCurrenttDenom + this->globaltransport_bbuBase;
    number currentBars = 0;
    number currentBeats = 0;
    number currentUnits = 0;

    if (targetBeatTime >= 0) {
        currentBars = trunc(targetBeatTime / numerator);
        targetBeatTime -= currentBars * numerator;
        currentBeats = trunc(targetBeatTime);
        targetBeatTime -= currentBeats;
        currentUnits = targetBeatTime * beatLengthInUnits;
    } else {
        currentBars = trunc(targetBeatTime / numerator);
        targetBeatTime -= currentBars * numerator;

        if (targetBeatTime != 0) {
            currentBars -= 1;
            currentBeats = trunc(targetBeatTime);
            targetBeatTime -= currentBeats;
            currentBeats = numerator + currentBeats;
            currentUnits = targetBeatTime * beatLengthInUnits;

            if (currentUnits != 0) {
                currentUnits = beatLengthInUnits + currentUnits;
                currentBeats -= 1;
            }
        }
    }

    return {currentBars + 1, currentBeats + 1, currentUnits};
}

bool globaltransport_setTimeSignature(MillisecondTime time, number numerator, number denominator, bool notify) {
    if ((bool)(notify)) {
        this->processTimeSignatureEvent(time, (int)(numerator), (int)(denominator));
        this->globaltransport_notify = true;
    } else {
        array<number, 2> currentSig = this->globaltransport_getTimeSignature(time);

        if (currentSig[0] != numerator || currentSig[1] != denominator) {
            array<number, 3> bbu = this->globaltransport_getBBU(time);
            this->globaltransport_setBBUBase(time, numerator, denominator, bbu[0], bbu[1], bbu[2]);
            this->globaltransport_timeSignatureChanges->push(numerator);
            this->globaltransport_timeSignatureChanges->push(denominator);
            this->globaltransport_timeSignatureChanges->push(time);
            return true;
        }
    }

    return false;
}

array<number, 3> globaltransport_getBBUAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getBBU(this->getEngine()->getCurrentTime() + msOffset);
}

bool globaltransport_setBBU(MillisecondTime time, number bars, number beats, number units, bool notify) {
    RNBO_UNUSED(notify);
    array<number, 2> sig = this->globaltransport_getTimeSignature(time);
    number numerator = sig[0];
    number denominator = sig[1];
    this->globaltransport_setBBUBase(time, numerator, denominator, bars, beats, units);
    return true;
}

void globaltransport_advance() {
    if ((bool)(this->globaltransport_tempoNeedsReset)) {
        fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
        this->globaltransport_tempoNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTempoEvent(this->globaltransport_lastTempo);
        }
    }

    if ((bool)(this->globaltransport_stateNeedsReset)) {
        fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
        this->globaltransport_stateNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTransportEvent(TransportState(this->globaltransport_lastState));
        }
    }

    if (this->globaltransport_beatTimeChanges->length > 2) {
        this->globaltransport_beatTimeChanges[0] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 2)];
        this->globaltransport_beatTimeChanges[1] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 1)];
        this->globaltransport_beatTimeChanges->length = 2;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendBeatTimeEvent(this->globaltransport_beatTimeChanges[0]);
        }
    }

    if (this->globaltransport_timeSignatureChanges->length > 3) {
        this->globaltransport_timeSignatureChanges[0] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 3)];
        this->globaltransport_timeSignatureChanges[1] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 2)];
        this->globaltransport_timeSignatureChanges[2] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 1)];
        this->globaltransport_timeSignatureChanges->length = 3;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTimeSignatureEvent(
                (int)(this->globaltransport_timeSignatureChanges[0]),
                (int)(this->globaltransport_timeSignatureChanges[1])
            );
        }
    }

    this->globaltransport_notify = false;
}

void globaltransport_dspsetup(bool force) {
    if ((bool)(this->globaltransport_setupDone) && (bool)(!(bool)(force)))
        return;

    fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
    this->globaltransport_tempoNeedsReset = false;
    fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
    this->globaltransport_stateNeedsReset = false;
    this->globaltransport_setupDone = true;
}

bool stackprotect_check() {
    this->stackprotect_count++;

    if (this->stackprotect_count > 128) {
        console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
        return true;
    }

    return false;
}

void updateTime(MillisecondTime time) {
    this->_currentTime = time;
    this->sampleOffsetIntoNextAudioBuffer = (SampleIndex)(rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr)));

    if (this->sampleOffsetIntoNextAudioBuffer >= (SampleIndex)(this->vs))
        this->sampleOffsetIntoNextAudioBuffer = (SampleIndex)(this->vs) - 1;

    if (this->sampleOffsetIntoNextAudioBuffer < 0)
        this->sampleOffsetIntoNextAudioBuffer = 0;
}

void assign_defaults()
{
    toggle_01_value_number = 0;
    toggle_01_value_number_setter(toggle_01_value_number);
    data_01_sizeout = 0;
    data_01_size = 0;
    data_01_sizems = 0;
    data_01_normalize = 0.995;
    data_01_channels = 2;
    data_02_sizeout = 0;
    data_02_size = 0;
    data_02_sizems = 0;
    data_02_normalize = 0.995;
    data_02_channels = 2;
    data_03_sizeout = 0;
    data_03_size = 0;
    data_03_sizems = 0;
    data_03_normalize = 0.995;
    data_03_channels = 2;
    data_04_sizeout = 0;
    data_04_size = 0;
    data_04_sizems = 0;
    data_04_normalize = 0.995;
    data_04_channels = 2;
    data_05_sizeout = 0;
    data_05_size = 0;
    data_05_sizems = 0;
    data_05_normalize = 0.995;
    data_05_channels = 2;
    data_06_sizeout = 0;
    data_06_size = 0;
    data_06_sizems = 0;
    data_06_normalize = 0.995;
    data_06_channels = 2;
    param_01_value = 0;
    pan_tilde_01_spread = 0.5;
    pan_tilde_01_pos = 0;
    data_07_sizeout = 0;
    data_07_size = 0;
    data_07_sizems = 0;
    data_07_normalize = 0.995;
    data_07_channels = 2;
    data_08_sizeout = 0;
    data_08_size = 0;
    data_08_sizems = 0;
    data_08_normalize = 0.995;
    data_08_channels = 2;
    groove_01_rate_auto = 1;
    groove_01_begin = 0;
    groove_01_end = -1;
    groove_01_loop = 1;
    groove_01_crossfade = 0;
    gaintilde_01_input_number = 0;
    gaintilde_01_value = 0;
    gaintilde_01_interp = 10;
    groove_02_rate_auto = 1;
    groove_02_begin = 0;
    groove_02_end = -1;
    groove_02_loop = 1;
    groove_02_crossfade = 0;
    toggle_02_value_number = 0;
    toggle_02_value_number_setter(toggle_02_value_number);
    random_01_limit = 1;
    random_01_limit_setter(random_01_limit);
    random_01_seed = 0;
    line_01_time = 0;
    line_01_grain = 20;
    line_01_output = 0;
    gaintilde_02_input_number = 0;
    gaintilde_02_value = 0;
    gaintilde_02_interp = 10;
    numberobj_01_value = 0;
    numberobj_01_value_setter(numberobj_01_value);
    dspexpr_01_in1 = 0;
    dspexpr_01_in2 = 1.2;
    pan_tilde_02_spread = 0.2;
    pan_tilde_02_pos = 0;
    groove_03_rate_auto = 1;
    groove_03_begin = 0;
    groove_03_end = -1;
    groove_03_loop = 0;
    groove_03_crossfade = 0;
    pan_tilde_03_spread = 0.5;
    pan_tilde_03_pos = 0;
    adsr_01_trigger_number = 0;
    adsr_01_attack = 0;
    adsr_01_decay = 0;
    adsr_01_sustain = 0;
    adsr_01_release = 0;
    adsr_01_legato = 0;
    adsr_01_maxsustain = -1;
    toggle_03_value_number = 0;
    toggle_03_value_number_setter(toggle_03_value_number);
    numberobj_02_value = 0;
    numberobj_02_value_setter(numberobj_02_value);
    counter_01_carryflag = 0;
    counter_01_direction = 0;
    counter_01_resetnext_number = 0;
    counter_01_resetnow_number = 0;
    counter_01_maximum = 7;
    counter_01_output = 0;
    counter_01_underflow_number = 0;
    counter_01_overflow_number = 0;
    counter_01_carry = 0;
    counter_01_minimum = 0;
    metro_01_on = 0;
    metro_01_interval = 0;
    metro_01_interval_setter(metro_01_interval);
    toggle_04_value_number = 0;
    toggle_04_value_number_setter(toggle_04_value_number);
    dspexpr_02_in1 = 0;
    dspexpr_02_in2 = 0;
    slider_01_value = 0;
    mtof_tilde_01_midivalue = 0;
    mtof_tilde_01_base = 440;
    cycle_tilde_01_frequency = 0;
    cycle_tilde_01_phase_offset = 0;
    line_02_time = 0;
    line_02_grain = 20;
    line_02_output = 0;
    dspexpr_03_in1 = 0;
    dspexpr_03_in2 = 1.2;
    slider_02_value = 0;
    expr_01_in1 = 0;
    expr_01_in2 = 0;
    expr_01_out1 = 0;
    numberobj_03_value = 0;
    numberobj_03_value_setter(numberobj_03_value);
    gaintilde_03_input_number = 0;
    gaintilde_03_value = 0;
    gaintilde_03_interp = 10;
    random_02_limit = 1;
    random_02_limit_setter(random_02_limit);
    random_02_seed = 0;
    line_03_time = 0;
    line_03_grain = 20;
    line_03_output = 0;
    transport_01_input_number = 0;
    transport_01_position = 0;
    transport_01_tempo = 120;
    transport_01_ticks = 0;
    line_04_time = 0;
    line_04_grain = 20;
    line_04_output = 0;
    numberobj_04_value = 0;
    numberobj_04_value_setter(numberobj_04_value);
    select_01_test1 = 0;
    select_01_test2 = 2;
    select_01_test3 = 4;
    select_01_test4 = 5;
    select_01_test5 = 6;
    gaintilde_04_input_number = 0;
    gaintilde_04_value = 0;
    gaintilde_04_interp = 10;
    numberobj_05_value = 0;
    numberobj_05_value_setter(numberobj_05_value);
    random_03_limit = 1;
    random_03_limit_setter(random_03_limit);
    random_03_seed = 0;
    slider_03_value = 0;
    mtof_tilde_02_midivalue = 0;
    mtof_tilde_02_base = 440;
    rect_tilde_01_frequency = 0;
    rect_tilde_01_pulsewidth = 0.5;
    rect_tilde_01_syncPhase = 0;
    dspexpr_04_in1 = 0;
    dspexpr_04_in2 = 0;
    slider_04_value = 0;
    line_05_time = 0;
    line_05_grain = 20;
    line_05_output = 0;
    numberobj_06_value = 0;
    numberobj_06_value_setter(numberobj_06_value);
    line_06_time = 0;
    line_06_grain = 20;
    line_06_output = 0;
    pan_tilde_04_spread = 0.5;
    pan_tilde_04_pos = 0;
    dspexpr_05_in1 = 0;
    dspexpr_05_in2 = 0;
    slider_05_value = 0;
    mtof_tilde_03_midivalue = 0;
    mtof_tilde_03_base = 440;
    saw_tilde_01_frequency = 0;
    saw_tilde_01_syncPhase = 0;
    dspexpr_06_in1 = 0;
    dspexpr_06_in2 = 0;
    slider_06_value = 0;
    line_07_time = 0;
    line_07_grain = 20;
    line_07_output = 0;
    select_02_test1 = 1;
    line_08_time = 0;
    line_08_grain = 20;
    line_08_output = 0;
    param_02_value = 0;
    multibuffertilde_01_current = 0;
    expr_02_in1 = 0;
    expr_02_in2 = 1;
    expr_02_out1 = 0;
    expr_03_in1 = 0;
    expr_03_in2 = 0.5;
    expr_03_out1 = 0;
    expr_04_in1 = 0;
    expr_04_in2 = 1;
    expr_04_out1 = 0;
    random_04_limit = 5;
    random_04_limit_setter(random_04_limit);
    random_04_seed = 0;
    numberobj_07_value = 0;
    numberobj_07_value_setter(numberobj_07_value);
    random_05_limit = 1;
    random_05_limit_setter(random_05_limit);
    random_05_seed = 0;
    line_09_time = 0;
    line_09_grain = 20;
    line_09_output = 0;
    line_10_time = 0;
    line_10_grain = 20;
    line_10_output = 0;
    numberobj_08_value = 0;
    numberobj_08_value_setter(numberobj_08_value);
    numberobj_09_value = 0;
    numberobj_09_value_setter(numberobj_09_value);
    phasor_01_freq = 0;
    triangle_tilde_01_phase = 0;
    triangle_tilde_01_duty = 0.5;
    slider_07_value = 0;
    mtof_tilde_04_midivalue = 0;
    mtof_tilde_04_base = 440;
    dspexpr_07_in1 = 0;
    dspexpr_07_in2 = 0;
    line_11_time = 0;
    line_11_grain = 20;
    line_11_output = 0;
    slider_08_value = 0;
    groove_04_rate_auto = 1;
    groove_04_begin = 0;
    groove_04_end = -1;
    groove_04_loop = 1;
    groove_04_crossfade = 0;
    toggle_05_value_number = 0;
    toggle_05_value_number_setter(toggle_05_value_number);
    pan_tilde_05_spread = 0.1;
    pan_tilde_05_pos = 0;
    gaintilde_05_input_number = 0;
    gaintilde_05_value = 0;
    gaintilde_05_interp = 10;
    line_12_time = 0;
    line_12_grain = 20;
    line_12_output = 0;
    numberobj_10_value = 0;
    numberobj_10_value_setter(numberobj_10_value);
    pan_tilde_06_spread = 0.5;
    pan_tilde_06_pos = 0;
    gaintilde_06_input_number = 0;
    gaintilde_06_value = 0;
    gaintilde_06_interp = 10;
    line_13_time = 0;
    line_13_grain = 20;
    line_13_output = 0;
    gaintilde_07_input_number = 0;
    gaintilde_07_value = 0;
    gaintilde_07_interp = 10;
    random_06_limit = 18;
    random_06_limit_setter(random_06_limit);
    random_06_seed = 0;
    numberobj_11_value = 0;
    numberobj_11_value_setter(numberobj_11_value);
    toggle_06_value_number = 0;
    toggle_06_value_number_setter(toggle_06_value_number);
    metro_02_on = 0;
    metro_02_interval = 0;
    metro_02_interval_setter(metro_02_interval);
    counter_02_carryflag = 0;
    counter_02_direction = 0;
    counter_02_resetnext_number = 0;
    counter_02_resetnow_number = 0;
    counter_02_maximum = 10;
    counter_02_output = 0;
    counter_02_underflow_number = 0;
    counter_02_overflow_number = 0;
    counter_02_carry = 0;
    counter_02_minimum = 0;
    expr_05_in1 = 0;
    expr_05_in2 = 0.2;
    expr_05_out1 = 0;
    random_07_limit = 0.3;
    random_07_limit_setter(random_07_limit);
    random_07_seed = 0;
    random_08_limit = 1;
    random_08_limit_setter(random_08_limit);
    random_08_seed = 0;
    line_14_time = 0;
    line_14_grain = 20;
    line_14_output = 0;
    switchobj_01_onoff = 0;
    gaintilde_08_input_number = 0;
    gaintilde_08_value = 0;
    gaintilde_08_interp = 10;
    select_03_test1 = 1;
    param_03_value = 0;
    numberobj_12_value = 0;
    numberobj_12_value_setter(numberobj_12_value);
    numberobj_13_value = 0;
    numberobj_13_value_setter(numberobj_13_value);
    param_04_value = 0.5;
    numberobj_14_value = 0;
    numberobj_14_value_setter(numberobj_14_value);
    numberobj_15_value = 0;
    numberobj_15_value_setter(numberobj_15_value);
    line_15_time = 0;
    line_15_grain = 20;
    line_15_output = 0;
    expr_06_in1 = 0;
    expr_06_in2 = 1;
    expr_06_out1 = 0;
    random_09_limit = 0.3;
    random_09_limit_setter(random_09_limit);
    random_09_seed = 0;
    numberobj_16_value = 0;
    numberobj_16_value_setter(numberobj_16_value);
    numberobj_17_value = 0;
    numberobj_17_value_setter(numberobj_17_value);
    line_16_time = 0;
    line_16_grain = 20;
    line_16_output = 0;
    counter_03_carryflag = 0;
    counter_03_direction = 0;
    counter_03_resetnext_number = 0;
    counter_03_resetnow_number = 0;
    counter_03_maximum = 1;
    counter_03_output = 0;
    counter_03_underflow_number = 0;
    counter_03_overflow_number = 0;
    counter_03_carry = 0;
    counter_03_minimum = 0;
    expr_07_in1 = 0;
    expr_07_in2 = 1;
    expr_07_out1 = 0;
    numberobj_18_value = 0;
    numberobj_18_value_setter(numberobj_18_value);
    delay_01_time = 120;
    delay_01_delayall = 1;
    random_10_limit = 18;
    random_10_limit_setter(random_10_limit);
    random_10_seed = 0;
    numberobj_19_value = 0;
    numberobj_19_value_setter(numberobj_19_value);
    toggle_07_value_number = 0;
    toggle_07_value_number_setter(toggle_07_value_number);
    metro_03_on = 0;
    metro_03_interval = 0;
    metro_03_interval_setter(metro_03_interval);
    counter_04_carryflag = 0;
    counter_04_direction = 0;
    counter_04_resetnext_number = 0;
    counter_04_resetnow_number = 0;
    counter_04_maximum = 10;
    counter_04_output = 0;
    counter_04_underflow_number = 0;
    counter_04_overflow_number = 0;
    counter_04_carry = 0;
    counter_04_minimum = 0;
    expr_08_in1 = 0;
    expr_08_in2 = 1;
    expr_08_out1 = 0;
    random_11_limit = 0.3;
    random_11_limit_setter(random_11_limit);
    random_11_seed = 0;
    switchobj_02_onoff = 0;
    numberobj_20_value = 0;
    numberobj_20_value_setter(numberobj_20_value);
    numberobj_21_value = 0;
    numberobj_21_value_setter(numberobj_21_value);
    numberobj_22_value = 0;
    numberobj_22_value_setter(numberobj_22_value);
    line_17_time = 0;
    line_17_grain = 20;
    line_17_output = 0;
    expr_09_in1 = 0;
    expr_09_in2 = 0.2;
    expr_09_out1 = 0;
    random_12_limit = 0.3;
    random_12_limit_setter(random_12_limit);
    random_12_seed = 0;
    numberobj_23_value = 0;
    numberobj_23_value_setter(numberobj_23_value);
    counter_05_carryflag = 0;
    counter_05_direction = 0;
    counter_05_resetnext_number = 0;
    counter_05_resetnow_number = 0;
    counter_05_maximum = 1;
    counter_05_output = 0;
    counter_05_underflow_number = 0;
    counter_05_overflow_number = 0;
    counter_05_carry = 0;
    counter_05_minimum = 0;
    expr_10_in1 = 0;
    expr_10_in2 = 1;
    expr_10_out1 = 0;
    numberobj_24_value = 0;
    numberobj_24_value_setter(numberobj_24_value);
    random_13_limit = 18;
    random_13_limit_setter(random_13_limit);
    random_13_seed = 0;
    numberobj_25_value = 0;
    numberobj_25_value_setter(numberobj_25_value);
    toggle_08_value_number = 0;
    toggle_08_value_number_setter(toggle_08_value_number);
    metro_04_on = 0;
    metro_04_interval = 0;
    metro_04_interval_setter(metro_04_interval);
    counter_06_carryflag = 0;
    counter_06_direction = 0;
    counter_06_resetnext_number = 0;
    counter_06_resetnow_number = 0;
    counter_06_maximum = 10;
    counter_06_output = 0;
    counter_06_underflow_number = 0;
    counter_06_overflow_number = 0;
    counter_06_carry = 0;
    counter_06_minimum = 0;
    expr_11_in1 = 0;
    expr_11_in2 = 0.2;
    expr_11_out1 = 0;
    random_14_limit = 0.3;
    random_14_limit_setter(random_14_limit);
    random_14_seed = 0;
    switchobj_03_onoff = 0;
    numberobj_26_value = 0;
    numberobj_26_value_setter(numberobj_26_value);
    numberobj_27_value = 0;
    numberobj_27_value_setter(numberobj_27_value);
    numberobj_28_value = 0;
    numberobj_28_value_setter(numberobj_28_value);
    line_18_time = 0;
    line_18_grain = 20;
    line_18_output = 0;
    expr_12_in1 = 0;
    expr_12_in2 = 1;
    expr_12_out1 = 0;
    random_15_limit = 0.3;
    random_15_limit_setter(random_15_limit);
    random_15_seed = 0;
    numberobj_29_value = 0;
    numberobj_29_value_setter(numberobj_29_value);
    counter_07_carryflag = 0;
    counter_07_direction = 0;
    counter_07_resetnext_number = 0;
    counter_07_resetnow_number = 0;
    counter_07_maximum = 1;
    counter_07_output = 0;
    counter_07_underflow_number = 0;
    counter_07_overflow_number = 0;
    counter_07_carry = 0;
    counter_07_minimum = 0;
    expr_13_in1 = 0;
    expr_13_in2 = 1;
    expr_13_out1 = 0;
    numberobj_30_value = 0;
    numberobj_30_value_setter(numberobj_30_value);
    random_16_limit = 18;
    random_16_limit_setter(random_16_limit);
    random_16_seed = 0;
    numberobj_31_value = 0;
    numberobj_31_value_setter(numberobj_31_value);
    toggle_09_value_number = 0;
    toggle_09_value_number_setter(toggle_09_value_number);
    metro_05_on = 0;
    metro_05_interval = 0;
    metro_05_interval_setter(metro_05_interval);
    counter_08_carryflag = 0;
    counter_08_direction = 0;
    counter_08_resetnext_number = 0;
    counter_08_resetnow_number = 0;
    counter_08_maximum = 10;
    counter_08_output = 0;
    counter_08_underflow_number = 0;
    counter_08_overflow_number = 0;
    counter_08_carry = 0;
    counter_08_minimum = 0;
    expr_14_in1 = 0;
    expr_14_in2 = 1;
    expr_14_out1 = 0;
    random_17_limit = 0.3;
    random_17_limit_setter(random_17_limit);
    random_17_seed = 0;
    switchobj_04_onoff = 0;
    numberobj_32_value = 0;
    numberobj_32_value_setter(numberobj_32_value);
    numberobj_33_value = 0;
    numberobj_33_value_setter(numberobj_33_value);
    numberobj_34_value = 0;
    numberobj_34_value_setter(numberobj_34_value);
    line_19_time = 0;
    line_19_grain = 20;
    line_19_output = 0;
    expr_15_in1 = 0;
    expr_15_in2 = 0.2;
    expr_15_out1 = 0;
    random_18_limit = 0.3;
    random_18_limit_setter(random_18_limit);
    random_18_seed = 0;
    numberobj_35_value = 0;
    numberobj_35_value_setter(numberobj_35_value);
    counter_09_carryflag = 0;
    counter_09_direction = 0;
    counter_09_resetnext_number = 0;
    counter_09_resetnow_number = 0;
    counter_09_maximum = 1;
    counter_09_output = 0;
    counter_09_underflow_number = 0;
    counter_09_overflow_number = 0;
    counter_09_carry = 0;
    counter_09_minimum = 0;
    expr_16_in1 = 0;
    expr_16_in2 = 1;
    expr_16_out1 = 0;
    numberobj_36_value = 0;
    numberobj_36_value_setter(numberobj_36_value);
    _currentTime = 0;
    audioProcessSampleCount = 0;
    sampleOffsetIntoNextAudioBuffer = 0;
    zeroBuffer = nullptr;
    dummyBuffer = nullptr;
    signals[0] = nullptr;
    signals[1] = nullptr;
    signals[2] = nullptr;
    signals[3] = nullptr;
    signals[4] = nullptr;
    signals[5] = nullptr;
    signals[6] = nullptr;
    signals[7] = nullptr;
    signals[8] = nullptr;
    signals[9] = nullptr;
    signals[10] = nullptr;
    signals[11] = nullptr;
    signals[12] = nullptr;
    signals[13] = nullptr;
    signals[14] = nullptr;
    didAllocateSignals = 0;
    vs = 0;
    maxvs = 0;
    sr = 44100;
    invsr = 0.00002267573696;
    toggle_01_lastValue = 0;
    data_01_sizemode = 0;
    data_01_setupDone = false;
    data_02_sizemode = 0;
    data_02_setupDone = false;
    data_03_sizemode = 0;
    data_03_setupDone = false;
    data_04_sizemode = 0;
    data_04_setupDone = false;
    data_05_sizemode = 0;
    data_05_setupDone = false;
    data_06_sizemode = 0;
    data_06_setupDone = false;
    param_01_lastValue = 0;
    data_07_sizemode = 0;
    data_07_setupDone = false;
    data_08_sizemode = 0;
    data_08_setupDone = false;
    groove_01_readIndex = 0;
    groove_01_playStatus = 0;
    groove_01_changeIncomingInSamples = 0;
    groove_01_incomingChange = 2;
    groove_01_crossfadeInSamples = 0;
    groove_01_setupDone = false;
    gaintilde_01_lastValue = 0;
    gaintilde_01_loginc = 1;
    gaintilde_01_zval = 0;
    gaintilde_01_ramp_d_prev = 0;
    gaintilde_01_ramp_prev = 0;
    gaintilde_01_ramp_index = 0;
    gaintilde_01_ramp_increment = 0;
    gaintilde_01_setupDone = false;
    groove_02_readIndex = 0;
    groove_02_playStatus = 0;
    groove_02_changeIncomingInSamples = 0;
    groove_02_incomingChange = 2;
    groove_02_crossfadeInSamples = 0;
    groove_02_setupDone = false;
    toggle_02_lastValue = 0;
    line_01_startTime = 0;
    line_01_startValue = 0.5;
    line_01_currentTarget = 0;
    line_01_slope = 0;
    line_01__time = 0;
    gaintilde_02_lastValue = 0;
    gaintilde_02_loginc = 1;
    gaintilde_02_zval = 0;
    gaintilde_02_ramp_d_prev = 0;
    gaintilde_02_ramp_prev = 0;
    gaintilde_02_ramp_index = 0;
    gaintilde_02_ramp_increment = 0;
    gaintilde_02_setupDone = false;
    numberobj_01_currentFormat = 6;
    numberobj_01_lastValue = 0;
    groove_03_readIndex = 0;
    groove_03_playStatus = 0;
    groove_03_changeIncomingInSamples = 0;
    groove_03_incomingChange = 2;
    groove_03_crossfadeInSamples = 0;
    groove_03_setupDone = false;
    adsr_01_phase = 3;
    adsr_01_mspersamp = 0;
    adsr_01_time = 0;
    adsr_01_lastTriggerVal = 0;
    adsr_01_amplitude = 0;
    adsr_01_outval = 0;
    adsr_01_startingpoint = 0;
    adsr_01_triggerBuf = nullptr;
    adsr_01_triggerValueBuf = nullptr;
    adsr_01_setupDone = false;
    toggle_03_lastValue = 0;
    numberobj_02_currentFormat = 6;
    numberobj_02_lastValue = 0;
    counter_01_count = 0;
    counter_01_phase = true;
    metro_01_last = -1;
    metro_01_next = -1;
    toggle_04_lastValue = 0;
    slider_01_lastValue = 0;
    mtof_tilde_01_innerMtoF_lastInValue = 0;
    mtof_tilde_01_innerMtoF_lastOutValue = 0;
    mtof_tilde_01_innerMtoF_lastTuning = 0;
    mtof_tilde_01_innerScala_lastValid = false;
    mtof_tilde_01_innerScala_lastNote = 0;
    mtof_tilde_01_innerScala_lastFreq = 0;
    mtof_tilde_01_innerScala_sclEntryCount = 0;
    mtof_tilde_01_innerScala_sclOctaveMul = 1;
    mtof_tilde_01_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_tilde_01_innerScala_kbmMid = 60;
    mtof_tilde_01_innerScala_kbmRefNum = 69;
    mtof_tilde_01_innerScala_kbmRefFreq = 440;
    mtof_tilde_01_innerScala_kbmSize = 0;
    mtof_tilde_01_innerScala_kbmMin = 0;
    mtof_tilde_01_innerScala_kbmMax = 0;
    mtof_tilde_01_innerScala_kbmOctaveDegree = 12;
    mtof_tilde_01_innerScala_kbmMapSize = 0;
    mtof_tilde_01_innerScala_refFreq = 261.63;
    cycle_tilde_01_wrap = 0;
    cycle_tilde_01_ph_currentPhase = 0;
    cycle_tilde_01_ph_conv = 0;
    cycle_tilde_01_setupDone = false;
    line_02_startTime = 0;
    line_02_startValue = 0;
    line_02_currentTarget = 0;
    line_02_slope = 0;
    line_02__time = 0;
    slider_02_lastValue = 0;
    numberobj_03_currentFormat = 6;
    numberobj_03_lastValue = 0;
    gaintilde_03_lastValue = 0;
    gaintilde_03_loginc = 1;
    gaintilde_03_zval = 0;
    gaintilde_03_ramp_d_prev = 0;
    gaintilde_03_ramp_prev = 0;
    gaintilde_03_ramp_index = 0;
    gaintilde_03_ramp_increment = 0;
    gaintilde_03_setupDone = false;
    line_03_startTime = 0;
    line_03_startValue = 0.5;
    line_03_currentTarget = 0;
    line_03_slope = 0;
    line_03__time = 0;
    line_04_startTime = 0;
    line_04_startValue = 0;
    line_04_currentTarget = 0;
    line_04_slope = 0;
    line_04__time = 0;
    numberobj_04_currentFormat = 6;
    numberobj_04_lastValue = 0;
    gaintilde_04_lastValue = 0;
    gaintilde_04_loginc = 1;
    gaintilde_04_zval = 0;
    gaintilde_04_ramp_d_prev = 0;
    gaintilde_04_ramp_prev = 0;
    gaintilde_04_ramp_index = 0;
    gaintilde_04_ramp_increment = 0;
    gaintilde_04_setupDone = false;
    numberobj_05_currentFormat = 6;
    numberobj_05_lastValue = 0;
    slider_03_lastValue = 0;
    mtof_tilde_02_innerMtoF_lastInValue = 0;
    mtof_tilde_02_innerMtoF_lastOutValue = 0;
    mtof_tilde_02_innerMtoF_lastTuning = 0;
    mtof_tilde_02_innerScala_lastValid = false;
    mtof_tilde_02_innerScala_lastNote = 0;
    mtof_tilde_02_innerScala_lastFreq = 0;
    mtof_tilde_02_innerScala_sclEntryCount = 0;
    mtof_tilde_02_innerScala_sclOctaveMul = 1;
    mtof_tilde_02_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_tilde_02_innerScala_kbmMid = 60;
    mtof_tilde_02_innerScala_kbmRefNum = 69;
    mtof_tilde_02_innerScala_kbmRefFreq = 440;
    mtof_tilde_02_innerScala_kbmSize = 0;
    mtof_tilde_02_innerScala_kbmMin = 0;
    mtof_tilde_02_innerScala_kbmMax = 0;
    mtof_tilde_02_innerScala_kbmOctaveDegree = 12;
    mtof_tilde_02_innerScala_kbmMapSize = 0;
    mtof_tilde_02_innerScala_refFreq = 261.63;
    rect_tilde_01_xHistory = 0;
    rect_tilde_01_yHistory = 0;
    rect_tilde_01_t = 0;
    rect_tilde_01_lastSyncPhase = 0;
    rect_tilde_01_lastSyncDiff = 0;
    rect_tilde_01_didSync = false;
    slider_04_lastValue = 0;
    line_05_startTime = 0;
    line_05_startValue = 0;
    line_05_currentTarget = 0;
    line_05_slope = 0;
    line_05__time = 0;
    numberobj_06_currentFormat = 6;
    numberobj_06_lastValue = 0;
    line_06_startTime = 0;
    line_06_startValue = 0;
    line_06_currentTarget = 0;
    line_06_slope = 0;
    line_06__time = 0;
    slider_05_lastValue = 0;
    mtof_tilde_03_innerMtoF_lastInValue = 0;
    mtof_tilde_03_innerMtoF_lastOutValue = 0;
    mtof_tilde_03_innerMtoF_lastTuning = 0;
    mtof_tilde_03_innerScala_lastValid = false;
    mtof_tilde_03_innerScala_lastNote = 0;
    mtof_tilde_03_innerScala_lastFreq = 0;
    mtof_tilde_03_innerScala_sclEntryCount = 0;
    mtof_tilde_03_innerScala_sclOctaveMul = 1;
    mtof_tilde_03_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_tilde_03_innerScala_kbmMid = 60;
    mtof_tilde_03_innerScala_kbmRefNum = 69;
    mtof_tilde_03_innerScala_kbmRefFreq = 440;
    mtof_tilde_03_innerScala_kbmSize = 0;
    mtof_tilde_03_innerScala_kbmMin = 0;
    mtof_tilde_03_innerScala_kbmMax = 0;
    mtof_tilde_03_innerScala_kbmOctaveDegree = 12;
    mtof_tilde_03_innerScala_kbmMapSize = 0;
    mtof_tilde_03_innerScala_refFreq = 261.63;
    saw_tilde_01_t = 0;
    saw_tilde_01_lastSyncPhase = 0;
    saw_tilde_01_lastSyncDiff = 0;
    saw_tilde_01_didSync = false;
    saw_tilde_01_dcblocker_xm1 = 0;
    saw_tilde_01_dcblocker_ym1 = 0;
    saw_tilde_01_setupDone = false;
    slider_06_lastValue = 0;
    line_07_startTime = 0;
    line_07_startValue = 0;
    line_07_currentTarget = 0;
    line_07_slope = 0;
    line_07__time = 0;
    line_08_startTime = 0;
    line_08_startValue = 0;
    line_08_currentTarget = 0;
    line_08_slope = 0;
    line_08__time = 0;
    param_02_lastValue = 0;
    numberobj_07_currentFormat = 6;
    numberobj_07_lastValue = 0;
    line_09_startTime = 0;
    line_09_startValue = 0.5;
    line_09_currentTarget = 0;
    line_09_slope = 0;
    line_09__time = 0;
    line_10_startTime = 0;
    line_10_startValue = 0;
    line_10_currentTarget = 0;
    line_10_slope = 0;
    line_10__time = 0;
    numberobj_08_currentFormat = 6;
    numberobj_08_lastValue = 0;
    numberobj_09_currentFormat = 6;
    numberobj_09_lastValue = 0;
    phasor_01_sigbuf = nullptr;
    phasor_01_lastLockedPhase = 0;
    phasor_01_conv = 0;
    phasor_01_ph_currentPhase = 0;
    phasor_01_ph_conv = 0;
    phasor_01_setupDone = false;
    slider_07_lastValue = 0;
    mtof_tilde_04_innerMtoF_lastInValue = 0;
    mtof_tilde_04_innerMtoF_lastOutValue = 0;
    mtof_tilde_04_innerMtoF_lastTuning = 0;
    mtof_tilde_04_innerScala_lastValid = false;
    mtof_tilde_04_innerScala_lastNote = 0;
    mtof_tilde_04_innerScala_lastFreq = 0;
    mtof_tilde_04_innerScala_sclEntryCount = 0;
    mtof_tilde_04_innerScala_sclOctaveMul = 1;
    mtof_tilde_04_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_tilde_04_innerScala_kbmMid = 60;
    mtof_tilde_04_innerScala_kbmRefNum = 69;
    mtof_tilde_04_innerScala_kbmRefFreq = 440;
    mtof_tilde_04_innerScala_kbmSize = 0;
    mtof_tilde_04_innerScala_kbmMin = 0;
    mtof_tilde_04_innerScala_kbmMax = 0;
    mtof_tilde_04_innerScala_kbmOctaveDegree = 12;
    mtof_tilde_04_innerScala_kbmMapSize = 0;
    mtof_tilde_04_innerScala_refFreq = 261.63;
    line_11_startTime = 0;
    line_11_startValue = 130;
    line_11_currentTarget = 0;
    line_11_slope = 0;
    line_11__time = 0;
    slider_08_lastValue = 0;
    groove_04_readIndex = 0;
    groove_04_playStatus = 0;
    groove_04_changeIncomingInSamples = 0;
    groove_04_incomingChange = 2;
    groove_04_crossfadeInSamples = 0;
    groove_04_setupDone = false;
    toggle_05_lastValue = 0;
    gaintilde_05_lastValue = 0;
    gaintilde_05_loginc = 1;
    gaintilde_05_zval = 0;
    gaintilde_05_ramp_d_prev = 0;
    gaintilde_05_ramp_prev = 0;
    gaintilde_05_ramp_index = 0;
    gaintilde_05_ramp_increment = 0;
    gaintilde_05_setupDone = false;
    line_12_startTime = 0;
    line_12_startValue = 0;
    line_12_currentTarget = 0;
    line_12_slope = 0;
    line_12__time = 0;
    numberobj_10_currentFormat = 6;
    numberobj_10_lastValue = 0;
    gaintilde_06_lastValue = 0;
    gaintilde_06_loginc = 1;
    gaintilde_06_zval = 0;
    gaintilde_06_ramp_d_prev = 0;
    gaintilde_06_ramp_prev = 0;
    gaintilde_06_ramp_index = 0;
    gaintilde_06_ramp_increment = 0;
    gaintilde_06_setupDone = false;
    line_13_startTime = 0;
    line_13_startValue = 0;
    line_13_currentTarget = 0;
    line_13_slope = 0;
    line_13__time = 0;
    gaintilde_07_lastValue = 0;
    gaintilde_07_loginc = 1;
    gaintilde_07_zval = 0;
    gaintilde_07_ramp_d_prev = 0;
    gaintilde_07_ramp_prev = 0;
    gaintilde_07_ramp_index = 0;
    gaintilde_07_ramp_increment = 0;
    gaintilde_07_setupDone = false;
    numberobj_11_currentFormat = 6;
    numberobj_11_lastValue = 0;
    toggle_06_lastValue = 0;
    metro_02_last = -1;
    metro_02_next = -1;
    counter_02_count = 0;
    counter_02_phase = true;
    line_14_startTime = 0;
    line_14_startValue = 0.5;
    line_14_currentTarget = 0;
    line_14_slope = 0;
    line_14__time = 0;
    gaintilde_08_lastValue = 0;
    gaintilde_08_loginc = 1;
    gaintilde_08_zval = 0;
    gaintilde_08_ramp_d_prev = 0;
    gaintilde_08_ramp_prev = 0;
    gaintilde_08_ramp_index = 0;
    gaintilde_08_ramp_increment = 0;
    gaintilde_08_setupDone = false;
    param_03_lastValue = 0;
    numberobj_12_currentFormat = 6;
    numberobj_12_lastValue = 0;
    numberobj_13_currentFormat = 6;
    numberobj_13_lastValue = 0;
    param_04_lastValue = 0;
    numberobj_14_currentFormat = 6;
    numberobj_14_lastValue = 0;
    numberobj_15_currentFormat = 6;
    numberobj_15_lastValue = 0;
    line_15_startTime = 0;
    line_15_startValue = 0.8;
    line_15_currentTarget = 0;
    line_15_slope = 0;
    line_15__time = 0;
    numberobj_16_currentFormat = 6;
    numberobj_16_lastValue = 0;
    numberobj_17_currentFormat = 6;
    numberobj_17_lastValue = 0;
    line_16_startTime = 0;
    line_16_startValue = 0;
    line_16_currentTarget = 0;
    line_16_slope = 0;
    line_16__time = 0;
    counter_03_count = 0;
    counter_03_phase = true;
    numberobj_18_currentFormat = 6;
    numberobj_18_lastValue = 0;
    numberobj_19_currentFormat = 6;
    numberobj_19_lastValue = 0;
    toggle_07_lastValue = 0;
    metro_03_last = -1;
    metro_03_next = -1;
    counter_04_count = 0;
    counter_04_phase = true;
    numberobj_20_currentFormat = 6;
    numberobj_20_lastValue = 0;
    numberobj_21_currentFormat = 6;
    numberobj_21_lastValue = 0;
    numberobj_22_currentFormat = 6;
    numberobj_22_lastValue = 0;
    line_17_startTime = 0;
    line_17_startValue = 0.8;
    line_17_currentTarget = 0;
    line_17_slope = 0;
    line_17__time = 0;
    numberobj_23_currentFormat = 6;
    numberobj_23_lastValue = 0;
    counter_05_count = 0;
    counter_05_phase = true;
    numberobj_24_currentFormat = 6;
    numberobj_24_lastValue = 0;
    numberobj_25_currentFormat = 6;
    numberobj_25_lastValue = 0;
    toggle_08_lastValue = 0;
    metro_04_last = -1;
    metro_04_next = -1;
    counter_06_count = 0;
    counter_06_phase = true;
    numberobj_26_currentFormat = 6;
    numberobj_26_lastValue = 0;
    numberobj_27_currentFormat = 6;
    numberobj_27_lastValue = 0;
    numberobj_28_currentFormat = 6;
    numberobj_28_lastValue = 0;
    line_18_startTime = 0;
    line_18_startValue = 0.8;
    line_18_currentTarget = 0;
    line_18_slope = 0;
    line_18__time = 0;
    numberobj_29_currentFormat = 6;
    numberobj_29_lastValue = 0;
    counter_07_count = 0;
    counter_07_phase = true;
    numberobj_30_currentFormat = 6;
    numberobj_30_lastValue = 0;
    numberobj_31_currentFormat = 6;
    numberobj_31_lastValue = 0;
    toggle_09_lastValue = 0;
    metro_05_last = -1;
    metro_05_next = -1;
    counter_08_count = 0;
    counter_08_phase = true;
    numberobj_32_currentFormat = 6;
    numberobj_32_lastValue = 0;
    numberobj_33_currentFormat = 6;
    numberobj_33_lastValue = 0;
    numberobj_34_currentFormat = 6;
    numberobj_34_lastValue = 0;
    line_19_startTime = 0;
    line_19_startValue = 0.8;
    line_19_currentTarget = 0;
    line_19_slope = 0;
    line_19__time = 0;
    numberobj_35_currentFormat = 6;
    numberobj_35_lastValue = 0;
    counter_09_count = 0;
    counter_09_phase = true;
    numberobj_36_currentFormat = 6;
    numberobj_36_lastValue = 0;
    globaltransport_tempo = nullptr;
    globaltransport_tempoNeedsReset = false;
    globaltransport_lastTempo = 120;
    globaltransport_state = nullptr;
    globaltransport_stateNeedsReset = false;
    globaltransport_lastState = 0;
    globaltransport_beatTimeChanges = { 0, 0 };
    globaltransport_timeSignatureChanges = { 4, 4, 0 };
    globaltransport_notify = false;
    globaltransport_bbuBase = 0;
    globaltransport_setupDone = false;
    stackprotect_count = 0;
    _voiceIndex = 0;
    _noteNumber = 0;
    isMuted = 1;
}

// member variables

    number toggle_01_value_number;
    number data_01_sizeout;
    number data_01_size;
    number data_01_sizems;
    number data_01_normalize;
    number data_01_channels;
    number data_02_sizeout;
    number data_02_size;
    number data_02_sizems;
    number data_02_normalize;
    number data_02_channels;
    list message_01_set;
    number data_03_sizeout;
    number data_03_size;
    number data_03_sizems;
    number data_03_normalize;
    number data_03_channels;
    number data_04_sizeout;
    number data_04_size;
    number data_04_sizems;
    number data_04_normalize;
    number data_04_channels;
    number data_05_sizeout;
    number data_05_size;
    number data_05_sizems;
    number data_05_normalize;
    number data_05_channels;
    number data_06_sizeout;
    number data_06_size;
    number data_06_sizems;
    number data_06_normalize;
    number data_06_channels;
    number param_01_value;
    number pan_tilde_01_spread;
    number pan_tilde_01_pos;
    number data_07_sizeout;
    number data_07_size;
    number data_07_sizems;
    number data_07_normalize;
    number data_07_channels;
    number data_08_sizeout;
    number data_08_size;
    number data_08_sizems;
    number data_08_normalize;
    number data_08_channels;
    number groove_01_rate_auto;
    number groove_01_begin;
    number groove_01_end;
    number groove_01_loop;
    number groove_01_crossfade;
    number gaintilde_01_input_number;
    number gaintilde_01_value;
    number gaintilde_01_interp;
    number groove_02_rate_auto;
    number groove_02_begin;
    number groove_02_end;
    number groove_02_loop;
    number groove_02_crossfade;
    number toggle_02_value_number;
    number random_01_limit;
    number random_01_seed;
    list line_01_target;
    number line_01_time;
    number line_01_grain;
    number line_01_output;
    number gaintilde_02_input_number;
    number gaintilde_02_value;
    number gaintilde_02_interp;
    number numberobj_01_value;
    list message_02_set;
    list message_03_set;
    number dspexpr_01_in1;
    number dspexpr_01_in2;
    number pan_tilde_02_spread;
    number pan_tilde_02_pos;
    number groove_03_rate_auto;
    number groove_03_begin;
    number groove_03_end;
    number groove_03_loop;
    number groove_03_crossfade;
    number pan_tilde_03_spread;
    number pan_tilde_03_pos;
    number adsr_01_trigger_number;
    number adsr_01_attack;
    number adsr_01_decay;
    number adsr_01_sustain;
    number adsr_01_release;
    number adsr_01_legato;
    number adsr_01_maxsustain;
    number toggle_03_value_number;
    number numberobj_02_value;
    Int counter_01_carryflag;
    number counter_01_direction;
    number counter_01_resetnext_number;
    number counter_01_resetnow_number;
    number counter_01_maximum;
    number counter_01_output;
    number counter_01_underflow_number;
    number counter_01_overflow_number;
    number counter_01_carry;
    number counter_01_minimum;
    number metro_01_on;
    number metro_01_interval;
    number toggle_04_value_number;
    number dspexpr_02_in1;
    number dspexpr_02_in2;
    number slider_01_value;
    number mtof_tilde_01_midivalue;
    list mtof_tilde_01_scale;
    list mtof_tilde_01_map;
    number mtof_tilde_01_base;
    number cycle_tilde_01_frequency;
    number cycle_tilde_01_phase_offset;
    list line_02_target;
    number line_02_time;
    number line_02_grain;
    number line_02_output;
    list message_04_set;
    number dspexpr_03_in1;
    number dspexpr_03_in2;
    number slider_02_value;
    number expr_01_in1;
    number expr_01_in2;
    number expr_01_out1;
    number numberobj_03_value;
    number gaintilde_03_input_number;
    number gaintilde_03_value;
    number gaintilde_03_interp;
    number random_02_limit;
    number random_02_seed;
    list line_03_target;
    number line_03_time;
    number line_03_grain;
    number line_03_output;
    list message_05_set;
    number transport_01_input_number;
    number transport_01_position;
    number transport_01_tempo;
    list transport_01_timesig;
    number transport_01_ticks;
    list line_04_target;
    number line_04_time;
    number line_04_grain;
    number line_04_output;
    list message_06_set;
    number numberobj_04_value;
    number select_01_test1;
    number select_01_test2;
    number select_01_test3;
    number select_01_test4;
    number select_01_test5;
    number gaintilde_04_input_number;
    number gaintilde_04_value;
    number gaintilde_04_interp;
    number numberobj_05_value;
    number random_03_limit;
    number random_03_seed;
    number slider_03_value;
    number mtof_tilde_02_midivalue;
    list mtof_tilde_02_scale;
    list mtof_tilde_02_map;
    number mtof_tilde_02_base;
    number rect_tilde_01_frequency;
    number rect_tilde_01_pulsewidth;
    number rect_tilde_01_syncPhase;
    number dspexpr_04_in1;
    number dspexpr_04_in2;
    list message_07_set;
    number slider_04_value;
    list line_05_target;
    number line_05_time;
    number line_05_grain;
    number line_05_output;
    list message_08_set;
    number numberobj_06_value;
    list line_06_target;
    number line_06_time;
    number line_06_grain;
    number line_06_output;
    list message_09_set;
    number pan_tilde_04_spread;
    number pan_tilde_04_pos;
    number dspexpr_05_in1;
    number dspexpr_05_in2;
    number slider_05_value;
    number mtof_tilde_03_midivalue;
    list mtof_tilde_03_scale;
    list mtof_tilde_03_map;
    number mtof_tilde_03_base;
    number saw_tilde_01_frequency;
    number saw_tilde_01_syncPhase;
    number dspexpr_06_in1;
    number dspexpr_06_in2;
    number slider_06_value;
    list message_10_set;
    list line_07_target;
    number line_07_time;
    number line_07_grain;
    number line_07_output;
    number select_02_test1;
    list message_11_set;
    list line_08_target;
    number line_08_time;
    number line_08_grain;
    number line_08_output;
    number param_02_value;
    number multibuffertilde_01_current;
    number expr_02_in1;
    number expr_02_in2;
    number expr_02_out1;
    number expr_03_in1;
    number expr_03_in2;
    number expr_03_out1;
    number expr_04_in1;
    number expr_04_in2;
    number expr_04_out1;
    number random_04_limit;
    number random_04_seed;
    number numberobj_07_value;
    number random_05_limit;
    number random_05_seed;
    list line_09_target;
    number line_09_time;
    number line_09_grain;
    number line_09_output;
    list message_12_set;
    list line_10_target;
    number line_10_time;
    number line_10_grain;
    number line_10_output;
    number numberobj_08_value;
    number numberobj_09_value;
    number phasor_01_freq;
    number triangle_tilde_01_phase;
    number triangle_tilde_01_duty;
    number slider_07_value;
    number mtof_tilde_04_midivalue;
    list mtof_tilde_04_scale;
    list mtof_tilde_04_map;
    number mtof_tilde_04_base;
    number dspexpr_07_in1;
    number dspexpr_07_in2;
    list line_11_target;
    number line_11_time;
    number line_11_grain;
    number line_11_output;
    list message_13_set;
    list message_14_set;
    number slider_08_value;
    number groove_04_rate_auto;
    number groove_04_begin;
    number groove_04_end;
    number groove_04_loop;
    number groove_04_crossfade;
    number toggle_05_value_number;
    number pan_tilde_05_spread;
    number pan_tilde_05_pos;
    number gaintilde_05_input_number;
    number gaintilde_05_value;
    number gaintilde_05_interp;
    list message_15_set;
    list line_12_target;
    number line_12_time;
    number line_12_grain;
    number line_12_output;
    number numberobj_10_value;
    list message_16_set;
    number pan_tilde_06_spread;
    number pan_tilde_06_pos;
    number gaintilde_06_input_number;
    number gaintilde_06_value;
    number gaintilde_06_interp;
    list message_17_set;
    list line_13_target;
    number line_13_time;
    number line_13_grain;
    number line_13_output;
    number gaintilde_07_input_number;
    number gaintilde_07_value;
    number gaintilde_07_interp;
    number random_06_limit;
    number random_06_seed;
    number numberobj_11_value;
    number toggle_06_value_number;
    number metro_02_on;
    number metro_02_interval;
    Int counter_02_carryflag;
    number counter_02_direction;
    number counter_02_resetnext_number;
    number counter_02_resetnow_number;
    number counter_02_maximum;
    number counter_02_output;
    number counter_02_underflow_number;
    number counter_02_overflow_number;
    number counter_02_carry;
    number counter_02_minimum;
    number expr_05_in1;
    number expr_05_in2;
    number expr_05_out1;
    number random_07_limit;
    number random_07_seed;
    number random_08_limit;
    number random_08_seed;
    list line_14_target;
    number line_14_time;
    number line_14_grain;
    number line_14_output;
    number switchobj_01_onoff;
    number gaintilde_08_input_number;
    number gaintilde_08_value;
    number gaintilde_08_interp;
    list message_18_set;
    number select_03_test1;
    number param_03_value;
    number numberobj_12_value;
    number numberobj_13_value;
    number param_04_value;
    number numberobj_14_value;
    list message_19_set;
    number numberobj_15_value;
    list line_15_target;
    number line_15_time;
    number line_15_grain;
    number line_15_output;
    number expr_06_in1;
    number expr_06_in2;
    number expr_06_out1;
    number random_09_limit;
    number random_09_seed;
    number numberobj_16_value;
    list message_20_set;
    number numberobj_17_value;
    list line_16_target;
    number line_16_time;
    number line_16_grain;
    number line_16_output;
    list message_21_set;
    Int counter_03_carryflag;
    number counter_03_direction;
    number counter_03_resetnext_number;
    number counter_03_resetnow_number;
    number counter_03_maximum;
    number counter_03_output;
    number counter_03_underflow_number;
    number counter_03_overflow_number;
    number counter_03_carry;
    number counter_03_minimum;
    number expr_07_in1;
    number expr_07_in2;
    number expr_07_out1;
    number numberobj_18_value;
    number delay_01_time;
    number delay_01_delayall;
    number random_10_limit;
    number random_10_seed;
    number numberobj_19_value;
    number toggle_07_value_number;
    number metro_03_on;
    number metro_03_interval;
    Int counter_04_carryflag;
    number counter_04_direction;
    number counter_04_resetnext_number;
    number counter_04_resetnow_number;
    number counter_04_maximum;
    number counter_04_output;
    number counter_04_underflow_number;
    number counter_04_overflow_number;
    number counter_04_carry;
    number counter_04_minimum;
    number expr_08_in1;
    number expr_08_in2;
    number expr_08_out1;
    number random_11_limit;
    number random_11_seed;
    number switchobj_02_onoff;
    number numberobj_20_value;
    number numberobj_21_value;
    number numberobj_22_value;
    list line_17_target;
    number line_17_time;
    number line_17_grain;
    number line_17_output;
    number expr_09_in1;
    number expr_09_in2;
    number expr_09_out1;
    number random_12_limit;
    number random_12_seed;
    number numberobj_23_value;
    list message_22_set;
    Int counter_05_carryflag;
    number counter_05_direction;
    number counter_05_resetnext_number;
    number counter_05_resetnow_number;
    number counter_05_maximum;
    number counter_05_output;
    number counter_05_underflow_number;
    number counter_05_overflow_number;
    number counter_05_carry;
    number counter_05_minimum;
    number expr_10_in1;
    number expr_10_in2;
    number expr_10_out1;
    number numberobj_24_value;
    number random_13_limit;
    number random_13_seed;
    number numberobj_25_value;
    number toggle_08_value_number;
    number metro_04_on;
    number metro_04_interval;
    Int counter_06_carryflag;
    number counter_06_direction;
    number counter_06_resetnext_number;
    number counter_06_resetnow_number;
    number counter_06_maximum;
    number counter_06_output;
    number counter_06_underflow_number;
    number counter_06_overflow_number;
    number counter_06_carry;
    number counter_06_minimum;
    number expr_11_in1;
    number expr_11_in2;
    number expr_11_out1;
    number random_14_limit;
    number random_14_seed;
    number switchobj_03_onoff;
    number numberobj_26_value;
    number numberobj_27_value;
    number numberobj_28_value;
    list line_18_target;
    number line_18_time;
    number line_18_grain;
    number line_18_output;
    number expr_12_in1;
    number expr_12_in2;
    number expr_12_out1;
    number random_15_limit;
    number random_15_seed;
    number numberobj_29_value;
    list message_23_set;
    Int counter_07_carryflag;
    number counter_07_direction;
    number counter_07_resetnext_number;
    number counter_07_resetnow_number;
    number counter_07_maximum;
    number counter_07_output;
    number counter_07_underflow_number;
    number counter_07_overflow_number;
    number counter_07_carry;
    number counter_07_minimum;
    number expr_13_in1;
    number expr_13_in2;
    number expr_13_out1;
    number numberobj_30_value;
    number random_16_limit;
    number random_16_seed;
    number numberobj_31_value;
    number toggle_09_value_number;
    number metro_05_on;
    number metro_05_interval;
    Int counter_08_carryflag;
    number counter_08_direction;
    number counter_08_resetnext_number;
    number counter_08_resetnow_number;
    number counter_08_maximum;
    number counter_08_output;
    number counter_08_underflow_number;
    number counter_08_overflow_number;
    number counter_08_carry;
    number counter_08_minimum;
    number expr_14_in1;
    number expr_14_in2;
    number expr_14_out1;
    number random_17_limit;
    number random_17_seed;
    number switchobj_04_onoff;
    number numberobj_32_value;
    number numberobj_33_value;
    number numberobj_34_value;
    list line_19_target;
    number line_19_time;
    number line_19_grain;
    number line_19_output;
    number expr_15_in1;
    number expr_15_in2;
    number expr_15_out1;
    number random_18_limit;
    number random_18_seed;
    number numberobj_35_value;
    list message_24_set;
    Int counter_09_carryflag;
    number counter_09_direction;
    number counter_09_resetnext_number;
    number counter_09_resetnow_number;
    number counter_09_maximum;
    number counter_09_output;
    number counter_09_underflow_number;
    number counter_09_overflow_number;
    number counter_09_carry;
    number counter_09_minimum;
    number expr_16_in1;
    number expr_16_in2;
    number expr_16_out1;
    number numberobj_36_value;
    MillisecondTime _currentTime;
    UInt64 audioProcessSampleCount;
    SampleIndex sampleOffsetIntoNextAudioBuffer;
    signal zeroBuffer;
    signal dummyBuffer;
    SampleValue * signals[15];
    bool didAllocateSignals;
    Index vs;
    Index maxvs;
    number sr;
    number invsr;
    number toggle_01_lastValue;
    Float32BufferRef data_01_buffer;
    Int data_01_sizemode;
    bool data_01_setupDone;
    Float32BufferRef data_02_buffer;
    Int data_02_sizemode;
    bool data_02_setupDone;
    Float32BufferRef data_03_buffer;
    Int data_03_sizemode;
    bool data_03_setupDone;
    Float32BufferRef data_04_buffer;
    Int data_04_sizemode;
    bool data_04_setupDone;
    Float32BufferRef data_05_buffer;
    Int data_05_sizemode;
    bool data_05_setupDone;
    Float32BufferRef data_06_buffer;
    Int data_06_sizemode;
    bool data_06_setupDone;
    number param_01_lastValue;
    Float32BufferRef data_07_buffer;
    Int data_07_sizemode;
    bool data_07_setupDone;
    Float32BufferRef data_08_buffer;
    Int data_08_sizemode;
    bool data_08_setupDone;
    Float32BufferRef groove_01_buffer;
    number groove_01_readIndex;
    Index groove_01_playStatus;
    SampleIndex groove_01_changeIncomingInSamples;
    Int groove_01_incomingChange;
    SampleIndex groove_01_crossfadeInSamples;
    bool groove_01_setupDone;
    number gaintilde_01_lastValue;
    number gaintilde_01_loginc;
    number gaintilde_01_zval;
    number gaintilde_01_ramp_d_prev;
    number gaintilde_01_ramp_prev;
    number gaintilde_01_ramp_index;
    number gaintilde_01_ramp_increment;
    bool gaintilde_01_setupDone;
    Float32BufferRef groove_02_buffer;
    number groove_02_readIndex;
    Index groove_02_playStatus;
    SampleIndex groove_02_changeIncomingInSamples;
    Int groove_02_incomingChange;
    SampleIndex groove_02_crossfadeInSamples;
    bool groove_02_setupDone;
    number toggle_02_lastValue;
    UInt random_01_nz_state[4] = { };
    MillisecondTime line_01_startTime;
    number line_01_startValue;
    number line_01_currentTarget;
    number line_01_slope;
    MillisecondTime line_01__time;
    list line_01_pendingRamps;
    number gaintilde_02_lastValue;
    number gaintilde_02_loginc;
    number gaintilde_02_zval;
    number gaintilde_02_ramp_d_prev;
    number gaintilde_02_ramp_prev;
    number gaintilde_02_ramp_index;
    number gaintilde_02_ramp_increment;
    bool gaintilde_02_setupDone;
    Int numberobj_01_currentFormat;
    number numberobj_01_lastValue;
    Float32MultiBufferRef groove_03_buffer;
    number groove_03_readIndex;
    Index groove_03_playStatus;
    SampleIndex groove_03_changeIncomingInSamples;
    Int groove_03_incomingChange;
    SampleIndex groove_03_crossfadeInSamples;
    bool groove_03_setupDone;
    Int adsr_01_phase;
    number adsr_01_mspersamp;
    number adsr_01_time;
    number adsr_01_lastTriggerVal;
    number adsr_01_amplitude;
    number adsr_01_outval;
    number adsr_01_startingpoint;
    signal adsr_01_triggerBuf;
    signal adsr_01_triggerValueBuf;
    bool adsr_01_setupDone;
    number toggle_03_lastValue;
    Int numberobj_02_currentFormat;
    number numberobj_02_lastValue;
    number counter_01_count;
    bool counter_01_phase;
    MillisecondTime metro_01_last;
    MillisecondTime metro_01_next;
    number toggle_04_lastValue;
    number slider_01_lastValue;
    number mtof_tilde_01_innerMtoF_lastInValue;
    number mtof_tilde_01_innerMtoF_lastOutValue;
    number mtof_tilde_01_innerMtoF_lastTuning;
    Float64BufferRef mtof_tilde_01_innerMtoF_buffer;
    const Index mtof_tilde_01_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_tilde_01_innerScala_lastValid;
    number mtof_tilde_01_innerScala_lastNote;
    number mtof_tilde_01_innerScala_lastFreq;
    int mtof_tilde_01_innerScala_sclEntryCount;
    number mtof_tilde_01_innerScala_sclOctaveMul;
    list mtof_tilde_01_innerScala_sclExpMul;
    list mtof_tilde_01_innerScala_kbmValid;
    int mtof_tilde_01_innerScala_kbmMid;
    int mtof_tilde_01_innerScala_kbmRefNum;
    number mtof_tilde_01_innerScala_kbmRefFreq;
    int mtof_tilde_01_innerScala_kbmSize;
    int mtof_tilde_01_innerScala_kbmMin;
    int mtof_tilde_01_innerScala_kbmMax;
    int mtof_tilde_01_innerScala_kbmOctaveDegree;
    Index mtof_tilde_01_innerScala_kbmMapSize;
    number mtof_tilde_01_innerScala_refFreq;
    Float64BufferRef cycle_tilde_01_buffer;
    long cycle_tilde_01_wrap;
    uint32_t cycle_tilde_01_phasei;
    SampleValue cycle_tilde_01_f2i;
    number cycle_tilde_01_ph_currentPhase;
    number cycle_tilde_01_ph_conv;
    bool cycle_tilde_01_setupDone;
    MillisecondTime line_02_startTime;
    number line_02_startValue;
    number line_02_currentTarget;
    number line_02_slope;
    MillisecondTime line_02__time;
    list line_02_pendingRamps;
    number slider_02_lastValue;
    Int numberobj_03_currentFormat;
    number numberobj_03_lastValue;
    number gaintilde_03_lastValue;
    number gaintilde_03_loginc;
    number gaintilde_03_zval;
    number gaintilde_03_ramp_d_prev;
    number gaintilde_03_ramp_prev;
    number gaintilde_03_ramp_index;
    number gaintilde_03_ramp_increment;
    bool gaintilde_03_setupDone;
    UInt random_02_nz_state[4] = { };
    MillisecondTime line_03_startTime;
    number line_03_startValue;
    number line_03_currentTarget;
    number line_03_slope;
    MillisecondTime line_03__time;
    list line_03_pendingRamps;
    MillisecondTime line_04_startTime;
    number line_04_startValue;
    number line_04_currentTarget;
    number line_04_slope;
    MillisecondTime line_04__time;
    list line_04_pendingRamps;
    Int numberobj_04_currentFormat;
    number numberobj_04_lastValue;
    number gaintilde_04_lastValue;
    number gaintilde_04_loginc;
    number gaintilde_04_zval;
    number gaintilde_04_ramp_d_prev;
    number gaintilde_04_ramp_prev;
    number gaintilde_04_ramp_index;
    number gaintilde_04_ramp_increment;
    bool gaintilde_04_setupDone;
    Int numberobj_05_currentFormat;
    number numberobj_05_lastValue;
    UInt random_03_nz_state[4] = { };
    number slider_03_lastValue;
    number mtof_tilde_02_innerMtoF_lastInValue;
    number mtof_tilde_02_innerMtoF_lastOutValue;
    number mtof_tilde_02_innerMtoF_lastTuning;
    Float64BufferRef mtof_tilde_02_innerMtoF_buffer;
    const Index mtof_tilde_02_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_tilde_02_innerScala_lastValid;
    number mtof_tilde_02_innerScala_lastNote;
    number mtof_tilde_02_innerScala_lastFreq;
    int mtof_tilde_02_innerScala_sclEntryCount;
    number mtof_tilde_02_innerScala_sclOctaveMul;
    list mtof_tilde_02_innerScala_sclExpMul;
    list mtof_tilde_02_innerScala_kbmValid;
    int mtof_tilde_02_innerScala_kbmMid;
    int mtof_tilde_02_innerScala_kbmRefNum;
    number mtof_tilde_02_innerScala_kbmRefFreq;
    int mtof_tilde_02_innerScala_kbmSize;
    int mtof_tilde_02_innerScala_kbmMin;
    int mtof_tilde_02_innerScala_kbmMax;
    int mtof_tilde_02_innerScala_kbmOctaveDegree;
    Index mtof_tilde_02_innerScala_kbmMapSize;
    number mtof_tilde_02_innerScala_refFreq;
    number rect_tilde_01_xHistory;
    number rect_tilde_01_yHistory;
    number rect_tilde_01_t;
    number rect_tilde_01_lastSyncPhase;
    number rect_tilde_01_lastSyncDiff;
    bool rect_tilde_01_didSync;
    number slider_04_lastValue;
    MillisecondTime line_05_startTime;
    number line_05_startValue;
    number line_05_currentTarget;
    number line_05_slope;
    MillisecondTime line_05__time;
    list line_05_pendingRamps;
    Int numberobj_06_currentFormat;
    number numberobj_06_lastValue;
    MillisecondTime line_06_startTime;
    number line_06_startValue;
    number line_06_currentTarget;
    number line_06_slope;
    MillisecondTime line_06__time;
    list line_06_pendingRamps;
    number slider_05_lastValue;
    number mtof_tilde_03_innerMtoF_lastInValue;
    number mtof_tilde_03_innerMtoF_lastOutValue;
    number mtof_tilde_03_innerMtoF_lastTuning;
    Float64BufferRef mtof_tilde_03_innerMtoF_buffer;
    const Index mtof_tilde_03_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_tilde_03_innerScala_lastValid;
    number mtof_tilde_03_innerScala_lastNote;
    number mtof_tilde_03_innerScala_lastFreq;
    int mtof_tilde_03_innerScala_sclEntryCount;
    number mtof_tilde_03_innerScala_sclOctaveMul;
    list mtof_tilde_03_innerScala_sclExpMul;
    list mtof_tilde_03_innerScala_kbmValid;
    int mtof_tilde_03_innerScala_kbmMid;
    int mtof_tilde_03_innerScala_kbmRefNum;
    number mtof_tilde_03_innerScala_kbmRefFreq;
    int mtof_tilde_03_innerScala_kbmSize;
    int mtof_tilde_03_innerScala_kbmMin;
    int mtof_tilde_03_innerScala_kbmMax;
    int mtof_tilde_03_innerScala_kbmOctaveDegree;
    Index mtof_tilde_03_innerScala_kbmMapSize;
    number mtof_tilde_03_innerScala_refFreq;
    number saw_tilde_01_t;
    number saw_tilde_01_lastSyncPhase;
    number saw_tilde_01_lastSyncDiff;
    bool saw_tilde_01_didSync;
    number saw_tilde_01_dcblocker_xm1;
    number saw_tilde_01_dcblocker_ym1;
    bool saw_tilde_01_setupDone;
    number slider_06_lastValue;
    MillisecondTime line_07_startTime;
    number line_07_startValue;
    number line_07_currentTarget;
    number line_07_slope;
    MillisecondTime line_07__time;
    list line_07_pendingRamps;
    MillisecondTime line_08_startTime;
    number line_08_startValue;
    number line_08_currentTarget;
    number line_08_slope;
    MillisecondTime line_08__time;
    list line_08_pendingRamps;
    number param_02_lastValue;
    Float32BufferRef multibuffertilde_01_buffer0;
    Float32BufferRef multibuffertilde_01_buffer1;
    Float32BufferRef multibuffertilde_01_buffer2;
    Float32BufferRef multibuffertilde_01_buffer3;
    Float32BufferRef multibuffertilde_01_buffer4;
    Float32MultiBufferRef multibuffertilde_01_buffer;
    UInt random_04_nz_state[4] = { };
    Int numberobj_07_currentFormat;
    number numberobj_07_lastValue;
    UInt random_05_nz_state[4] = { };
    MillisecondTime line_09_startTime;
    number line_09_startValue;
    number line_09_currentTarget;
    number line_09_slope;
    MillisecondTime line_09__time;
    list line_09_pendingRamps;
    MillisecondTime line_10_startTime;
    number line_10_startValue;
    number line_10_currentTarget;
    number line_10_slope;
    MillisecondTime line_10__time;
    list line_10_pendingRamps;
    Int numberobj_08_currentFormat;
    number numberobj_08_lastValue;
    Int numberobj_09_currentFormat;
    number numberobj_09_lastValue;
    signal phasor_01_sigbuf;
    number phasor_01_lastLockedPhase;
    number phasor_01_conv;
    number phasor_01_ph_currentPhase;
    number phasor_01_ph_conv;
    bool phasor_01_setupDone;
    number slider_07_lastValue;
    number mtof_tilde_04_innerMtoF_lastInValue;
    number mtof_tilde_04_innerMtoF_lastOutValue;
    number mtof_tilde_04_innerMtoF_lastTuning;
    Float64BufferRef mtof_tilde_04_innerMtoF_buffer;
    const Index mtof_tilde_04_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_tilde_04_innerScala_lastValid;
    number mtof_tilde_04_innerScala_lastNote;
    number mtof_tilde_04_innerScala_lastFreq;
    int mtof_tilde_04_innerScala_sclEntryCount;
    number mtof_tilde_04_innerScala_sclOctaveMul;
    list mtof_tilde_04_innerScala_sclExpMul;
    list mtof_tilde_04_innerScala_kbmValid;
    int mtof_tilde_04_innerScala_kbmMid;
    int mtof_tilde_04_innerScala_kbmRefNum;
    number mtof_tilde_04_innerScala_kbmRefFreq;
    int mtof_tilde_04_innerScala_kbmSize;
    int mtof_tilde_04_innerScala_kbmMin;
    int mtof_tilde_04_innerScala_kbmMax;
    int mtof_tilde_04_innerScala_kbmOctaveDegree;
    Index mtof_tilde_04_innerScala_kbmMapSize;
    number mtof_tilde_04_innerScala_refFreq;
    MillisecondTime line_11_startTime;
    number line_11_startValue;
    number line_11_currentTarget;
    number line_11_slope;
    MillisecondTime line_11__time;
    list line_11_pendingRamps;
    number slider_08_lastValue;
    Float32BufferRef groove_04_buffer;
    number groove_04_readIndex;
    Index groove_04_playStatus;
    SampleIndex groove_04_changeIncomingInSamples;
    Int groove_04_incomingChange;
    SampleIndex groove_04_crossfadeInSamples;
    bool groove_04_setupDone;
    number toggle_05_lastValue;
    number gaintilde_05_lastValue;
    number gaintilde_05_loginc;
    number gaintilde_05_zval;
    number gaintilde_05_ramp_d_prev;
    number gaintilde_05_ramp_prev;
    number gaintilde_05_ramp_index;
    number gaintilde_05_ramp_increment;
    bool gaintilde_05_setupDone;
    MillisecondTime line_12_startTime;
    number line_12_startValue;
    number line_12_currentTarget;
    number line_12_slope;
    MillisecondTime line_12__time;
    list line_12_pendingRamps;
    Int numberobj_10_currentFormat;
    number numberobj_10_lastValue;
    number gaintilde_06_lastValue;
    number gaintilde_06_loginc;
    number gaintilde_06_zval;
    number gaintilde_06_ramp_d_prev;
    number gaintilde_06_ramp_prev;
    number gaintilde_06_ramp_index;
    number gaintilde_06_ramp_increment;
    bool gaintilde_06_setupDone;
    MillisecondTime line_13_startTime;
    number line_13_startValue;
    number line_13_currentTarget;
    number line_13_slope;
    MillisecondTime line_13__time;
    list line_13_pendingRamps;
    number gaintilde_07_lastValue;
    number gaintilde_07_loginc;
    number gaintilde_07_zval;
    number gaintilde_07_ramp_d_prev;
    number gaintilde_07_ramp_prev;
    number gaintilde_07_ramp_index;
    number gaintilde_07_ramp_increment;
    bool gaintilde_07_setupDone;
    UInt random_06_nz_state[4] = { };
    Int numberobj_11_currentFormat;
    number numberobj_11_lastValue;
    number toggle_06_lastValue;
    MillisecondTime metro_02_last;
    MillisecondTime metro_02_next;
    number counter_02_count;
    bool counter_02_phase;
    UInt random_07_nz_state[4] = { };
    UInt random_08_nz_state[4] = { };
    MillisecondTime line_14_startTime;
    number line_14_startValue;
    number line_14_currentTarget;
    number line_14_slope;
    MillisecondTime line_14__time;
    list line_14_pendingRamps;
    number gaintilde_08_lastValue;
    number gaintilde_08_loginc;
    number gaintilde_08_zval;
    number gaintilde_08_ramp_d_prev;
    number gaintilde_08_ramp_prev;
    number gaintilde_08_ramp_index;
    number gaintilde_08_ramp_increment;
    bool gaintilde_08_setupDone;
    number param_03_lastValue;
    Int numberobj_12_currentFormat;
    number numberobj_12_lastValue;
    Int numberobj_13_currentFormat;
    number numberobj_13_lastValue;
    number param_04_lastValue;
    Int numberobj_14_currentFormat;
    number numberobj_14_lastValue;
    Int numberobj_15_currentFormat;
    number numberobj_15_lastValue;
    MillisecondTime line_15_startTime;
    number line_15_startValue;
    number line_15_currentTarget;
    number line_15_slope;
    MillisecondTime line_15__time;
    list line_15_pendingRamps;
    UInt random_09_nz_state[4] = { };
    Int numberobj_16_currentFormat;
    number numberobj_16_lastValue;
    Int numberobj_17_currentFormat;
    number numberobj_17_lastValue;
    MillisecondTime line_16_startTime;
    number line_16_startValue;
    number line_16_currentTarget;
    number line_16_slope;
    MillisecondTime line_16__time;
    list line_16_pendingRamps;
    number counter_03_count;
    bool counter_03_phase;
    Int numberobj_18_currentFormat;
    number numberobj_18_lastValue;
    UInt random_10_nz_state[4] = { };
    Int numberobj_19_currentFormat;
    number numberobj_19_lastValue;
    number toggle_07_lastValue;
    MillisecondTime metro_03_last;
    MillisecondTime metro_03_next;
    number counter_04_count;
    bool counter_04_phase;
    UInt random_11_nz_state[4] = { };
    Int numberobj_20_currentFormat;
    number numberobj_20_lastValue;
    Int numberobj_21_currentFormat;
    number numberobj_21_lastValue;
    Int numberobj_22_currentFormat;
    number numberobj_22_lastValue;
    MillisecondTime line_17_startTime;
    number line_17_startValue;
    number line_17_currentTarget;
    number line_17_slope;
    MillisecondTime line_17__time;
    list line_17_pendingRamps;
    UInt random_12_nz_state[4] = { };
    Int numberobj_23_currentFormat;
    number numberobj_23_lastValue;
    number counter_05_count;
    bool counter_05_phase;
    Int numberobj_24_currentFormat;
    number numberobj_24_lastValue;
    UInt random_13_nz_state[4] = { };
    Int numberobj_25_currentFormat;
    number numberobj_25_lastValue;
    number toggle_08_lastValue;
    MillisecondTime metro_04_last;
    MillisecondTime metro_04_next;
    number counter_06_count;
    bool counter_06_phase;
    UInt random_14_nz_state[4] = { };
    Int numberobj_26_currentFormat;
    number numberobj_26_lastValue;
    Int numberobj_27_currentFormat;
    number numberobj_27_lastValue;
    Int numberobj_28_currentFormat;
    number numberobj_28_lastValue;
    MillisecondTime line_18_startTime;
    number line_18_startValue;
    number line_18_currentTarget;
    number line_18_slope;
    MillisecondTime line_18__time;
    list line_18_pendingRamps;
    UInt random_15_nz_state[4] = { };
    Int numberobj_29_currentFormat;
    number numberobj_29_lastValue;
    number counter_07_count;
    bool counter_07_phase;
    Int numberobj_30_currentFormat;
    number numberobj_30_lastValue;
    UInt random_16_nz_state[4] = { };
    Int numberobj_31_currentFormat;
    number numberobj_31_lastValue;
    number toggle_09_lastValue;
    MillisecondTime metro_05_last;
    MillisecondTime metro_05_next;
    number counter_08_count;
    bool counter_08_phase;
    UInt random_17_nz_state[4] = { };
    Int numberobj_32_currentFormat;
    number numberobj_32_lastValue;
    Int numberobj_33_currentFormat;
    number numberobj_33_lastValue;
    Int numberobj_34_currentFormat;
    number numberobj_34_lastValue;
    MillisecondTime line_19_startTime;
    number line_19_startValue;
    number line_19_currentTarget;
    number line_19_slope;
    MillisecondTime line_19__time;
    list line_19_pendingRamps;
    UInt random_18_nz_state[4] = { };
    Int numberobj_35_currentFormat;
    number numberobj_35_lastValue;
    number counter_09_count;
    bool counter_09_phase;
    Int numberobj_36_currentFormat;
    number numberobj_36_lastValue;
    signal globaltransport_tempo;
    bool globaltransport_tempoNeedsReset;
    number globaltransport_lastTempo;
    signal globaltransport_state;
    bool globaltransport_stateNeedsReset;
    number globaltransport_lastState;
    list globaltransport_beatTimeChanges;
    list globaltransport_timeSignatureChanges;
    bool globaltransport_notify;
    number globaltransport_bbuBase;
    bool globaltransport_setupDone;
    number stackprotect_count;
    DataRef background;
    DataRef b_hit1_wav;
    DataRef bigbot;
    DataRef littlebot;
    DataRef b_hit3_wav;
    DataRef b_hit2_wav;
    DataRef b_hit4_wav;
    DataRef b_hit5_wav;
    MultiDataRef drumchoice;
    DataRef RNBODefaultMtofLookupTable256;
    DataRef RNBODefaultSinus;
    Index _voiceIndex;
    Int _noteNumber;
    Index isMuted;
    indexlist paramInitIndices;
    indexlist paramInitOrder;

};

PatcherInterface* creaternbomatic()
{
    return new rnbomatic();
}

#ifndef RNBO_NO_PATCHERFACTORY

extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction(PlatformInterface* platformInterface)
#else

extern "C" PatcherFactoryFunctionPtr rnbomaticFactoryFunction(PlatformInterface* platformInterface)
#endif

{
    Platform::set(platformInterface);
    return creaternbomatic;
}

} // end RNBO namespace

