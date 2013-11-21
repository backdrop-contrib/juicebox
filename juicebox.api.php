<?php


/**
 * @file
 * Hooks provided by the Juicebox module.
 */


/**
 * Allow modules to alter the Juicebox gallery object used to build gallery
 * embed code and and XML.
 *
 * @param object $juicebox
 *   A Juicebox connector object that contains the gallery ($juicebox->gallery)
 *   which is going to be rendered. The full object is available for context but
 *   by this point the only alterations that will have any effect are those
 *   applied to $juicebox->gallery.
 * @param mixed $data
 *   The raw Drupal data that was used to build this gallery. Provided for
 *   context.
 */
function hook_juicebox_gallery_alter($juicebox, $data) {
  // See if this is a gallery sourced from a view.
  if ($juicebox->type === 'view') {
    $view = $data;
    // Assume we have a view called "galleries" and a page called "page_1" that
    // structures galleries based on a taxonomy term contextual filter. We want
    // the juicebox "galleryDescription" option to be the term description, but
    // because this term description is dynamic (based on contextual filter) we
    // can't statically define it in the view's Juicebox settings. This hook
    // let's us do the job dynamically.
    if ($view->name == 'galleries' && $view->current_display == 'page_1') {
      if (!empty($view->args)) {
        $term = taxonomy_term_load($view->args[0]);
        if (!empty($term->description)) {
          $juicebox->gallery->addOption('gallerydescription', strip_tags($term->description));
        }
      }
    }
  }
}


/**
 * Allow modules to alter the classes that are instantiated when a Juicebox
 * connector is created (pseudo plugin behavior).
 *
 * @param array $classes
 *   An associative array containing the class names that will be instantiated:
 *   - connector_plugin: The Juicebox connector plugin (based on the abstract
 *     base class JuiceboxConnector). This contains the building blocks for
 *     structuring Drupal data into a Juicebox gallery.
 *   - gallery: A gallery object dependency (implementing
 *     JuiceboxGalleryInterface) that's used to create the script and markup
 *     outputs of a Juicebox gallery.
 * @param array $xml_path_args
 *   An indexed array of XML path arguments that describe this gallery (and
 *   make up its XML URL). This information uniquely identifies the gallery and
 *   contains all the descriptive data needed to (re)build it. Typically the
 *   first arg on this array contains the 'type' identifier.
 * @param array $context
 *   Additional context variables for reference:
 *   - data: Loaded Drupal data that the gallery can be built from. Typically
 *     NULL when instantiating a gallery during an XML request.
 *   - library: Juicebox javascript library data as provided through Libraries
 *     API.
 * 
 * @see juicebox()
 */
function hook_juicebox_classes_alter(&$classes, $xml_path_args, $context) {
  // Completely swap-out the connector used to build field-based galleries.
  if ($xml_path_args[0] == 'field') {
    $classes['connector_plugin'] = 'MyBetterCustomFieldConnector';
  }
  // Add a new connector to create a Juicebox display formatter for some kind
  // of Drupal element not already supported by this module.
  if ($xml_path_args[0] == 'juicy_drupal_widget') {
    $classes['connector_plugin'] = 'JuiceboxConnectorJuicyWidget';
  }
  // Swap out the gallery dependency object because some future Juicebox
  // javascript library has a different interface.
  if (!empty($context['library']['version']) && $context['library']['version'] == 'Pro 12.3') {
    $classes['gallery'] = 'FutureJuiceboxGallery';
  }
}
