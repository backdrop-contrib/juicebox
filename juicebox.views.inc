<?php

/**
 * @file
 * Views integration for Juicebox module.
 */


/**
 * Implements hook_views_plugin().
 */
function juicebox_views_plugins() {
  $path = drupal_get_path('module', 'juicebox');
  $plugins['style']['juicebox'] = array(
    'title' => t('Juicebox Gallery'),
    'help' => t('Display rows as a Juicebox Gallery.'),
    'handler' => 'juicebox_style_plugin',
    'theme' => 'juicebox_view',
    'path' => $path . '/plugins',
     // views requires a separate file for theme functions
     // see also: http://drupal.org/node/593336
    'theme file' => 'juicebox.theme.inc',
    'theme path' => $path . '/themes',
    'uses row plugin' => FALSE,
    'uses fields' => TRUE,
    'uses options' => TRUE,
    'uses grouping' => FALSE,
    'type' => 'normal',
  );
  return $plugins;
}
