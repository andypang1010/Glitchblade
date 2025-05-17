# SDL App Display Extensions
---
This directory is a collection of extensions that we have found necessary for CUGL.  In particular, mobile devices need support for safe area queries and device orientation (as opposed to display orienation).

Note that these features require modifications to the Android Java files (notably `SDLActivity`). Instead of modifying the SDL files, we subclass them with a new activity called `APPActivity`.  See my comments in that file in the Android project template.