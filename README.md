# Juicebox

The Juicebox module provides an integration between the Juicebox HTML5
responsive gallery library (http://juicebox.net/) and Backdrop. 
Juicebox is in many ways the successor of Simpleviewer and offers 
a powerful and flexible image gallery front-end.

The Juicebox library is downloaded, maintained and supported totally
independently from this Backdrop project. Both the Lite (free) and Pro 
versions should work with this module, and the one you choose will 
depend largely on how much formatting flexibly you require.

This is a port to Backrop of the Drupal project, version 7.x-2.1.
Please see the Drupal project page for a detailed feature overview:
http://drupal.org/project/juicebox

## Installation

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.
   
- Also install the Libraries module for Backdrop.

- Download the Juicebox javascript library from 
  http://juicebox.net/download/ , (the Juicebox-lite version is free) 
  and extract the contents of folder web/jbcore and add them
  to your sites libraries folder in a subdirectory 'juicebox'.

## Configuration and Use

This module integrates with Backdrop on many levels but conceptually 
it operates just like any other display formatter. It is designed to let
you easily turn groups of Backdrop-managed image data into Juicebox 
galleries without making too many assumptions about how your site is 
structured or what media management strategy you use.

If you want to add galleries to individual nodes/entities you can use 
the Juicebox field formatter (this is probably the easiest option to 
setup and manage). Or, if you need to group image data from multiple 
nodes/entities/files into galleries, and leverage the flexibility of 
views to organize everything, you can use the Juicebox views style plugin.

Step-by-step directions and additional information can be found in the 
module documentation at http://drupal.org/node/2000300, which includes 
dedicated pages for the field formatter and the views style plugin.


## License

This project is GPL v2 software. See the LICENSE.txt file 
in this directory for complete text.

## Current Maintainers for Backdrop

 + Graham Oliver (github.com/Graham-72/)
 + Ryan Jacobs (github.com/ryan-jacobs)


## Credits

This is a port of the Drupal project Juicebox, version 7.x-2.1

### Drupal Author and Maintainer

 + Ryan Jacobs (rjacobs) (github: ryan-jacobs)
 
### Simpleviewer Inc

 + the providers of the Juicebox javascript library
   for HTML5 Responsive Image Galleries.
