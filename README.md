# Glitchblade

Glitchblade features fast-paced, reflex-driven combat, where players must master enemy attack patterns and environmental hazards through precise timing and tactical movement. It combines responsive controls and mechanics with a high-skill ceiling, offering a truly rewarding combat experience. The game challenges players to adapt and react in real time, making every encounter engaging and intense.

## Glitchblade with Max MSP + RNBO, made by George

### New Features

Play the first level of the game, and you should hear the following new features:

- The background music, consisting of various basic waves, pans and changes in volume in a totally random way.
- The percussion sounds (including the stone hitting and the glitchy hit) are chosen randomly from a pool of hit sounds.
- As the level progresses, the background music will change gradually and a new layer of sound will be added (the one that is very glitchy).
- When the final enemy appears, it carries a bass line and a machine voice. Its sound will be panned according to its relative position to the player.

### How to Build

I am writing this as a reference on how to build this project. Max MSP is a visual sound engineering software that kinda follows the same logic as Unreal's Blueprints. RNBO is a tool that allows you to export Max patches to C++ code (or other formats) to be used elsewhere. This branch now already contains the Max patcher I made that generates the sound for in-game (in RNBO/source_patcher). the RNBO/rnbo-test2.cpp is the generateed C++ code by RNBO. It also contains the code that integrates the RNBO with CUGL: you can find it in GBGameScene.cpp.

I have only successfully built the project on MacOS.
To build the project, first use `python cugl .` (or whatever) to link cugl with the game. Then open XCode. Then, drag the entire RNBO folder into the XCode project. Make sure to select `Reference... in place` and `Create groups`. I've only tried building the project for Mac (so not for iOS or Android). Then, go to Build Settings, and select the `Header Search Paths`. Add the RNBO folder AND the RNBO/rnbo/common folder. For the RNBO folder select "recursive". For the RNBO/rnbo/common folder select "non-recursive". Then, go to `Build Phases` then `Compile Sources`. Add rnbo-test2.cpp and RNBO/rnbo/RNBO.cpp. Then, go to `Copy Bundle Resources` and add the RNBO/dependencies.json. Now, you should be able to build the project.

For more questions or help please contact me via georgezou52@gmail.com

### Supported Platforms

- Windows
- MacOS
- iOS
- Android
