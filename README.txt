
General Information
-------------------

This module provides an integration between the Juicebox HTML5 responsive 
gallery library (http://juicebox.net/) and Drupal. Juicebox is in many ways the
successor of Simpleviewer and offers a powerful and flexible image gallery 
front-end.


The Juicebox library (downloaded separately) is:

* Based on HTML5 (no flash) and supports "universal playback", meaning it can
run on nearly any device.

* Fully responsive so galleries can be made to dynamically re-size and adapt to
different browser sizes.

* Powered by a lightweight javascript library.

* Free (though not open source). A "Pro" version is also available that adds
some special features.


This Juicebox integration module offers:

* A field formatter that allows image fields to be formatted as Juicebox
Galleries. This allows individual nodes/entities to quickly be structured as
full Juicebox galleries.

* A views style plugin that allows arbitrary field-based views to be presented
as Juicebox galleries. This means nodes/entities can be setup to store
individual images and then presented as Juicebox galleries via views (with all
the filtering and sorting possibilities of views).

* Full control over nearly all Juicebox-specific configuration options.


Drupal module author:
Ryan Jacobs (rjacobs)
http://drupal.org/user/422459
http://www.ryan-jacobs.net



Installation
------------

1. Install and enable the required Libraries API module from
http://drupal.org/project/libraries.

2. Download the 3rd party Juicebox library from 
http://www.juicebox.net/download/ and extract it to a temporary location.

3. Copy the core Juicebox library files to your site's library directory.
Typically this means you will create a new directory called
/sites/all/libraries/juicebox and then copy the CONTENTS of the "jbcore"
directory (that you extracted in the previous step) to this directory. You will
end up with /sites/all/libraries/juicebox/juicebox.js ... etc.

4. Install and enable this Juicebox module.

If for any reason you enable the Juicebox module (step 4) BEFORE installing the
Juicebox library (step 3), or if you make changes to the Juicebox library
itself, please be sure to clear your Drupal cache at
/admin/config/development/performance. This will ensure that the correct
Juicebox library information is detected by the Libraries API.



Usage and Configuration
-----------------------

This module does not introduce any new structural or storage concepts to Drupal,
it simply offers a new way to display groups of images that you already maintain
via "standard" Drupal techniques. This means that its usage and configuration
depends on what methods you prefer to use for the storage and organization of
your image data. If you prefer to setup individual nodes/entities as full
galleries, you can use the Juicebox field formatter (this is perhaps the easiest
option to setup and manage). If you prefer to setup individual nodes/entities as
images, and then use all the organizational flexibility of views to structure
and display your galleries, you can use the Juicebox views style plugin (this is
certainly the most flexible option).


**** Using the Juicebox field formatter (nodes/entities are galleries): ****

1. Create a content type to represent a full gallery/album object. Using the
"manage fields" tab add any fields to this type that you would like but be sure
to include an "image" field that will be used to hold all the images that will
be part of each gallery (one image field can hold multiple images). In the image
field options be sure to:

* Check the box to "enable alt field" (this can be used later to hold caption or
title information).

* Check the box to "enable title field" (this can be used later to hold caption
or title information).

* For "number of values" be sure you select a value greater than one, such as
"unlimited".  This will allow you to add multiple images to a single node/entity
(thus making a gallery).


2. Using the "manage display" tab, find the entry for your image field and under
the "format" options choose "Juicebox Gallery". Then click on the "gear" button
to the right to open the Juicebox-specific display options. Here you can:

* Specify a width and height for the gallery.

* Specify which image styles to use when displaying images and thumbnails. Note
that you first may need to create a new style at
/admin/config/media/image-styles if none of the available options are suitable.

* Specify which parts of the image field data to use as caption and title
sources (if any).

* Specify any additional Juicebox configuration options for this gallery
(see: http://juicebox.net/support/config_options/).


3. For each gallery that you would like to create, add a new node of the type
you just created. Upload the various images that you want for each gallery into
image field that you setup and formatted in the steps above.


**** Using the Juicebox views style plugin (nodes/entities are images): ****

1. Create a content type to represent each image that will be part of a gallery.
Add fields for the image itself (each node will only hold one image), a
text/html caption, a title, etc. You may also want to add a field that can be
used as a sorting parameter (such as a simple integer field), add taxonomy
reference field to "group" your images in albums/galleries, etc.

2. Add your images to your site using the content type you just created.

3. Create a new view that displays the content type that you created. Add a
standard page display to this view that uses the display format of "Juicebox
Gallery".

4. Be sure that you add a separate field to your view (from the content type
fields that you created earlier) for the image that you will display, the title
text that will accompany each image and the caption text that will accompany
each image.

5. Setup whatever content filters, sorting options, etc. that you like
(e.g. perhaps you want to use a taxonomy-based contextual filter to setup
distinct albums within this single view definition, etc.).

6. Under "format" click "settings" to access the Juicebox-specific display
options. Here you can:

* Specify a width and height for the gallery.

* Specify which of your view fields should be used for each of the Juicebox
gallery image elements (image, thumbnail, title, caption). You should have
already added a field for each to your view and will simply need to map each
of these options to the appropriate view field.

* Specify which image styles to use when displaying images and thumbnails. Note
that you first may need to create a new style at
/admin/config/media/image-styles if none of the available options are suitable.

* Specify any additional Juicebox configuration options for this gallery
(see: http://juicebox.net/support/config_options/).


7. Save your view. Note that the preview function in the view admin may not
display anything, you will need to navigate to the actual view path to see/test
the results.
