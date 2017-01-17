/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/numpy/processors/numpymandelbrot.h>
#include <modules/numpy/numpymodule.h>
#include <python3/pythoninterface/pyvalueparser.h>
#include <modules/numpy/numpyobjectwrapper.h>
#include <modules/python3/pybindutils.h>

namespace inviwo {


const ProcessorInfo NumpyMandelbrot::processorInfo_{
    "org.inviwo.NumpyMandelbrot",  // Class identifier
    "NumPy Mandelbrot",            // Display name
    "NumPy",                       // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo NumpyMandelbrot::getProcessorInfo() const {
    return processorInfo_;
}

NumpyMandelbrot::NumpyMandelbrot()
    : Processor()
    , outport_("outport" , false)
    , size_("size", "size_", size2_t(600, 440), size2_t(32), size2_t(2048)) 
    , realBounds_("realBounds", "Real bounds" , -2 , 1 , -3 , 3 )
    , imaginaryBound_("imaginaryBound", "Imaginary bounds", -1.1f, 1.1f, -3 , 3)
    , power_("power","power" , 2 , 0.01 , 10 , 0.01f)
    , iterations_("iterations","Iterations" , 10 , 1 , 1000)

    , script_(InviwoApplication::getPtr()->getModuleByType<NumPyModule>()->getPath(ModulePath::Scripts) + "/mandelbrot.py")
{
    
    addPort(outport_);
    addProperty(size_);
    addProperty(realBounds_);
    addProperty(imaginaryBound_);
    addProperty(power_);
    addProperty(iterations_);


    script_.onChange([&](){invalidate(InvalidationLevel::InvalidOutput);});

}
    
void NumpyMandelbrot::process() {
    auto img = std::make_shared<Image>( size_.get(), DataFloat32::get() );
    auto imgPy = util::toNumPyObject(img);
    script_.run({
      {"img" , imgPy->getPyBindObject()}
    , {"real" , pybind11::cast(realBounds_.get())}
    , {"im" , pybind11::cast(imaginaryBound_.get())}
    , {"power" , pybind11::cast(power_.get())}
    , {"N" , pybind11::cast(iterations_.get())}
    });

    outport_.setData(img);

}

} // namespace

