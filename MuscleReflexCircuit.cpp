/* -------------------------------------------------------------------------- *
 *                    OpenSim: MuscleReflexCircuit.cpp                        *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2021 Stanford University, TU Delft and the Authors      *
 * Author(s): Ajay Seth, Hjalti Hilmarsson                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied    *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */



//=============================================================================
// INCLUDES
//=============================================================================
#include "MuscleReflexCircuit.h"
#include <OpenSim/OpenSim.h>


// This allows us to use OpenSim functions, classes, etc., without having to
// prefix the names of those things with "OpenSim::".
using namespace OpenSim;
using namespace std;
using namespace SimTK;


//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/* Default constructor. */
MuscleReflexCircuit::MuscleReflexCircuit()
{
    constructProperties();
}

/* Convenience constructor. */
MuscleReflexCircuit::MuscleReflexCircuit(const std::string& name,
                                         const Muscle& muscle,
                                         const SimpleSpindle& spindle,
                                         const GolgiTendon& golgi,
                                         double threshold,
                                         double timeDelay,
                                         double defaultControlSignal)
{
    OPENSIM_THROW_IF(name.empty(), ComponentHasNoName, getClassName());
       
    setName(name);
    connectSocket_muscle(muscle);
    connectSocket_spindle(spindle);
    connectSocket_golgi(golgi);
    
    constructProperties();
    set_threshold(threshold);
    set_timeDelay(timeDelay);
    set_defaultControlSignal(defaultControlSignal);


}

//=============================================================================
// SETUP PROPERTIES
//=============================================================================
//
/* Connect properties to local pointers.
 *
 * Properties should be given a meaningful name and an informative comment.
 * The name you give each property is the tag that will be used in the XML
 * file. The comment will appear before the property in the XML file.
 * In addition, the comments are used for tool tips in the OpenSim GUI.
 *
 * All properties are added to the property set. Once added, they can be
 * read in and written to files.
 ____________________________________________________________________________


 * Construct Properties
 */
void MuscleReflexCircuit::constructProperties()
{
    constructProperty_defaultControlSignal(1.0);
    constructProperty_timeDelay(0.1);
    constructProperty_threshold(0.5);
    constructProperty_weights();
}


void MuscleReflexCircuit::extendConnectToModel(Model& model)
{
    Super::extendConnectToModel(model);
    
    // connect inputs to outputs here
    
    const SimpleSpindle spindle = getSpindle();
    const GolgiTendon golgi = getGolgi();
     
    Interneuron& interneuron = updInterneuron();
    Delay& delay = updDelay();
    
    // connect the interneuron inputs to the spindle and golgi outputs
    interneuron.updInput("afferents").connect(spindle.getOutput("spindle_length"));
    interneuron.updInput("afferents").connect(spindle.getOutput("spindle_speed"));
    interneuron.updInput("afferents").connect(golgi.getOutput("golgiLength"));
    
    // Connect the delay component input to the interneuron output
    delay.updInput("signal").connect(interneuron.getOutput("signal"));

}

void MuscleReflexCircuit::extendFinalizeFromProperties()
{
    Super::extendFinalizeFromProperties();
    
    
    //const double& weight[4] = {0,1,2,3};
    const auto& weights = getProperty_weights();
    
    Interneuron& interneuron = upd_Interneuron();
    Delay& delay = upd_Delay();
    
    // Setup the property values for the interneuron and delay component
    interneuron.setWeights(weights);   //  kíkja á const Property<double> (getWeights functionið, kannski hægt að gera bara Array<double> í staðinn)
    interneuron.setThreshold(get_threshold());
    delay.setDelayValue(get_timeDelay());
    delay.setDefaultSignal(get_defaultControlSignal());
    
    
    OPENSIM_THROW_IF_FRMOBJ(get_timeDelay() < SimTK::Eps, InvalidPropertyValue, getName(), "Delay value cannot be less than SimTK::Eps, if it is we throw the delay component from the simulation");
     
}


// create finalizefromproperties and check there for delay (use eps as lowest number of delay)

//=============================================================================
// GET AND SET
//=============================================================================

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------

Delay& MuscleReflexCircuit::updDelay()
{
    return upd_Delay();
}
const Delay& MuscleReflexCircuit::getDelay() const
{
    return get_Delay();
}

Interneuron& MuscleReflexCircuit::updInterneuron()
{
    return upd_Interneuron();
}
const Interneuron& MuscleReflexCircuit::getInterneuron() const
{
    return get_Interneuron();
}
//-----------------------------------------------------------------------------
// SOCKETS
//-----------------------------------------------------------------------------
const Muscle& MuscleReflexCircuit::getMuscle() const
{
    return getSocket<Muscle>("muscle").getConnectee();
}

const SimpleSpindle& MuscleReflexCircuit::getSpindle() const
{
    return getSocket<SimpleSpindle>("spindle").getConnectee();
}

const GolgiTendon& MuscleReflexCircuit::getGolgi() const
{
    return getSocket<GolgiTendon>("golgi").getConnectee();
}



//=============================================================================
// SIGNALS
//=============================================================================
//_____________________________________________________________________________
/**
 * Compute the signals after a delay /tau
 *
 * @param s         current state of the system
 */

double MuscleReflexCircuit::getMuscleSignal(const SimTK::State &s) const
{
    double muscle_signal = 0;
     
    const Delay delay = getDelay();
    
    muscle_signal = delay.getSignal(s);
    
    return muscle_signal;

}



