<?php


/**
 * @file
 * Theme related functions for processing output related to Juicebox galleries.
 */


/**
 * Returns HTML for embedding a Juicebox gallery.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
 */
function theme_juicebox_embed_markup(&$variables) {
  // Create a gallery id from the XML path
  $output = "<script>" . "\n";
  $output .= "  new juicebox({" . "\n";
  $output .= "    configUrl : '" . $variables['gallery_xml_path'] . "'," . "\n";
  $output .= "    containerId : '" . $variables['gallery_id'] . "'," . "\n";
  $output .= "    galleryWidth : '" . check_plain($variables['settings']['jlib_galleryWidth']) . "'," . "\n";
  $output .= "    galleryHeight : '" . check_plain($variables['settings']['jlib_galleryHeight']) . "'" . "\n";
  $output .= "  });" . "\n";
  $output .= "</script>" . "\n";
  $custom_classes = '';
  if (!empty($variables['settings']['custom_parent_classes'])) {
    $custom_classes = ' ' . check_plain(trim($variables['settings']['custom_parent_classes']));
  }
  $output .= '<div class="juicebox-parent' . $custom_classes . '">' . "\n";
  $output .= '<div id="' . $variables['gallery_id'] . '" class="juicebox-container"></div>' . "\n";
  $output .= '</div>' . "\n";
  return $output;
}
