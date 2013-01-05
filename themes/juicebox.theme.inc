<?php

/**
 * @file
 * Theme related functions for processing our output style plugins.
 *
 * Views bug: http://drupal.org/node/593336
 */

// Views templage preprocess function for our juicebox style view. This function
// renders the Juicebox XML, caches it, and then sets the variabels needed to
// generate the Juicebox embed markup (which is what this view displays).
function template_preprocess_juicebox_view(&$variables) {
  global $base_url;
  $cache_lifetime = variable_get('cache_lifetime', 0);
  dpm($cache_lifetime, "cache lifetime");
  $view = $variables['view'];
  // Render the Juicebox "confix.xml" for this view
  $xml = juicebox_render_xml_from_view($view);
  // Generate a unique view ID that can be used to label the XML in the cache
  $cid = '/view/' . $view->name . '/' . $view->current_display;
  foreach ($view->args as $arg) {
    $view_id .= '/' . $arg;
  }
  // Cache the xml in our "cache_juicebox" table. This XML will be fethched
  // via a separate URL within the Juicebox embed markup, so it only really 
  // needs to sit in the cache long enough for the embed html to be rendered. 
  // Our cached value will refresh itself each time the embed code is rendered 
  // so the expiration time should not really matter, but to be safe we set
  // it to match the core system cache lifetime. This should allow it to persist
  // at least as long as any cached embed code would persist in cache_page. Of
  // course if the embed code is cached in cache_page, the full XML page request 
  // should also be (further bypassing the need for this data in 
  // cache_juicebox), but why not be safe hey?
  cache_set($cid, $xml, 'cache_juicebox', time() + variable_get('cache_lifetime', 0));
  // Load the juicebox library, this will include the appropriate js in the 
  // page header, etc.
  $juicebox_library = libraries_load('juicebox');
  // Set templage variables for embed markup/
  $variables['juicebox_path'] = '';
  if (!empty($juicebox_library['library path'])) {
    $variables['juicebox_path'] = $juicebox_library['library path'];
  }
  $variables['config_url_path'] = $base_url . '/juicebox/xml' . $cid;
  $variables['style_options'] = $view->style_options;
}

