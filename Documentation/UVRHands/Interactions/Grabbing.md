
# Grabbing 

## Manus hands
Here the grabbing interaction is invoke
   The main way to invoke the grab is by pinching, so the thumb and the fingertips are close together(configurable threshold but default 3cm).  
   Unreal calculates every frame the distances between the fingers and the thumb and when the distance (which can be Configured inside the VRHand->CollisionObserver Component) falls below the threshold, a grab is triggered.
    This only works when animated hands (currently Manus Hands and SteamIndexController) are used. 
    

## Steam Index Controllers
Input Event-based     
When enabled grabbing and releasing items is done solely event-based wich means that an item is grabbed when an grab event defined in the project inputs is invoked.

Like for the Manus hands, finger proximity is also possible for the Steam Index Controller since the hands are also animated in VR and the grab is based on the animation.


## HTC Controllers
Input Event-based     
When enabled grabbing and releasing items is done solely event-based wich means that an item is grabbed when an grab event defined in the project inputs is invoked.
For this controller the default is pressing the trigger.



[Back To Main Page](../README.md)
