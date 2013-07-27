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
  // Create the gallery embed markup. This is what the Juicebox javascript will
  // use to generate the final gallery markup.
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
  $output .= '<div id="' . $variables['gallery_id'] . '" class="juicebox-container">' . "\n";

  // Also create a search engine friendly version of the gallery. This markup
  // will only be seen by crawlers and users with javascript disabled.
  $bypass_noscript = (array_key_exists('addseocontent', $variables['data']['jlib_options']) && $variables['data']['jlib_options']['addseocontent'] == 'FALSE');
  if (!$bypass_noscript) {
    $output .= '<noscript>' . "\n";
    $output .= '<!-- Image gallery content for non-javascript devices -->' . "\n";
    if (array_key_exists('gallerytitle', $variables['data']['jlib_options'])) {
      $output .= '<h1 class="jb-name">' . $variables['data']['jlib_options']['gallerytitle'] . '</h1>' . "\n";
    }
    if (array_key_exists('gallerydescription', $variables['data']['jlib_options'])) {
      $output .= '<p class="jb-description">' . $variables['data']['jlib_options']['gallerydescription'] . '</p>' . "\n";
    }
    foreach ($variables['data']['images'] as $image) {
      $img_vars = array();
      $img_vars['path'] = $image['image_src'];
      if (!empty($image['title'])) {
        $img_vars['title'] = $image['title'];
        $image['title'] = '<span class="jb-title">' . $image['title'] . '</span><br/>';
      }
      if (!empty($image['caption'])) {
        $img_vars['alt'] = $image['caption'];
        $image['caption'] = '<span class="jb-caption">' . $image['caption'] . '</span>';
      }
      $output .= '<p class="jb-image">' . theme('image', $img_vars) . '<br/>' . $image['title'] . $image['caption'] . '</p>' . "\n";
    }
    $output .= '</noscript>' . "\n";
  }
  $output .= '</div></div>' . "\n";

  return $output;
}
