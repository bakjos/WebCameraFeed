Web Camera plugin for unreal engine 
===================================

A plugin that let's you use a web camera on Windows, using VideoInput library (direct show), Mac and IOS, using the AVFoundation Library and Android using the Android Camera API through JNI


## Features:
   - A widget component named 'WebCameraWidget' to view the camera feed from a User Interface Blue Print
   - An Actor Component named 'WebCameraComponent' to generate the texture and use it from any Actor
   - Both components allow to select the device, request a desired width and height and mirroring using a Global Shader 

## How to use

1. The plugin was tested on Unreal 4.18.3
2. You can clone the repository to `<Game>/Plugins/` or if you want you can use git submodules to your own git repository. Alternatively, you can copy to the `Engine/Plugins/` if you wish to make the plugin available to all of your projects.
   > Do not forget to run UE4's `Generate Project Files` to account for these changes!
3. If you want to use the widget component you can follow the first two steps from the [Quick Start Tutorial](hhttps://docs.unrealengine.com/latest/INT/Engine/UMG/QuickStart/2/index.html) but instead of create the text controls you can create a WebCameraWidget inside the Web Camera Category, inside the widget attributes you can choose requested width and height

![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/WebCameraCategory.png?raw=true)

![Camera Settings](https://github.com/bakjos/WebCameraFeed/blob/master/docs/WebCameraSettings.png?raw=true)

4. If you want to use the actor component you'll need to do the following:
	- Create a material with a TextureSamplerParameter2D parameter
		
		![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/Material.png?raw=true)

		![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/TextureSamplerParameter2D.png?raw=true)
		

	- Create a blue print actor and add a Web Camera component
	  
	  ![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/BluePrintActorComponent.png?raw=true)

	 - Put inside the creation script the dynamic material creation and assign the web camera texture to the TextureSamplerParameter2D parameter
	  
	  ![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/BluePrintActor.png?raw=true)

  	- When you're using the mobile version, is necessary to add inside the tick event of the actor the code to set the texture parameter again

	  ![Camera Category](https://github.com/bakjos/WebCameraFeed/blob/master/docs/BluePrintActor_Tick.png?raw=true)

	- For IOS is necessary to add the following line to the `Project Settings > IOS > Extra PList Data>Additional PList Data ` :
        `<key>NSCameraUsageDescription</key><string>Camera is used for AR functionality</string>`

	  A sample material and a sample blue print actor are inside the Content folder