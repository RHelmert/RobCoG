The Manus VRPawn has some new configuration options displayed in

![](Documentation/RH/VRPawnOptions.jpg)

The different checkboxes do the following:
GloveType defines which controller should be displayed and used. 
The options are Manus, SteamIndexController, and HTC Controller and the option changes the appearance and some functions of the motion controller used.

It is adviced to change the option to match the current controller.

The option ShowHands allows to display virtual hands. 
It is recommended to turn on this feature while using the Manus hands but can also be ticked when using controllers. 
Currently using hands is the only way to interactt with the drawers. 

This function can be combined with the show controller tickbox as wished. 
All combinations are possible but at least one should be enabled. 

The UseEventBasedGrab option changes the grab detection mechanism.
When enabled grabbing and releasing items is done solely event-based wich means that an item is grabbed when an grab event is thrown.

Disabling this function only works when animated hands (currently Manus Hands and SteamIndexController) (!Does not work with HTC controller atm) are used. 
It calculates every frame the distances between the fingers and the thumb and when the distance (which can be Configured inside the VRPawnManusHandsBP->CollisionObserver Component) falls below the threshold, a grab is triggered.

At the moment this works best with the SteamIndexControllers. In the future when the Manus Hands work well, they could use this too for more natural grabbing experiences. However, since of some inaccuracies using the Manus hands causes objects to be released unintentionally. Further implementations and optimizations are needed in this case. 

The Visuals of the Controller should not be changed at all.