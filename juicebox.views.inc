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
    'path' => $path . '/plugins',
    'uses row plugin' => FALSE,
    'uses fields' => TRUE,
    'uses options' => TRUE,
    'uses grouping' => FALSE,
    'type' => 'normal',
  );
  return $plugins;
}
