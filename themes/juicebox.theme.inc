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
  $settings = $variables['settings'];
  $output = '';
  // Create the gallery embed markup. This is what the Juicebox javascript will
  // use to generate the final gallery markup.
  $custom_classes = '';
  if (!empty($settings['custom_parent_classes'])) {
    $custom_classes = ' ' . check_plain(trim($settings['custom_parent_classes']));
  }
  $output .= '<div class="juicebox-parent' . $custom_classes . '">' . "\n";
  // Add debug toggle links if configured to display them.
  if (!empty($settings['show_debug_links'])) {
    $output .= theme('juicebox_toggle_embed_debug_markup', array('gallery_id' => $variables['gallery_id'], 'settings' => $settings, 'debug' => FALSE));
  }
  $output .= '<div id="' . $variables['gallery_id'] . '" class="juicebox-container">' . "\n";
  // Also create a search engine friendly version of the gallery. This markup
  // will only be seen by crawlers and users with javascript disabled.
  $bypass_noscript = (array_key_exists('addseocontent', $variables['gallery_options']) && $variables['gallery_options']['addseocontent'] == 'FALSE');
  if (!$bypass_noscript) {
    $output .= '<noscript>' . "\n";
    $output .= '<!-- Image gallery content for non-javascript devices -->' . "\n";
    if (array_key_exists('gallerytitle', $variables['gallery_options'])) {
      $output .= '<h1 class="jb-name">' . $variables['gallery_options']['gallerytitle'] . '</h1>' . "\n";
    }
    if (array_key_exists('gallerydescription', $variables['gallery_options'])) {
      $output .= '<p class="jb-description">' . $variables['gallery_options']['gallerydescription'] . '</p>' . "\n";
    }
    foreach ($variables['gallery_images'] as $image) {
      $img_vars = array();
      $img_vars['path'] = $image['src_data']['image_url'];
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


/**
 * Proprocess logic for the juicebox_debug_markup theme function.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
 */
function juicebox_preprocess_juicebox_debug_markup(&$variables) {
  // Calculate the XML link that will be displayed. This should not have the
  // "checksum" or "jb-debug" query strings and should be absolute.
  if (!empty($variables['xml']['options']['query'])) {
    unset($variables['xml']['options']['query']['checksum']);
    unset($variables['xml']['options']['query']['jb-debug']);
  }
  $variables['xml']['options']['absolute'] = TRUE;
  $xml_url = url($variables['xml']['path'], $variables['xml']['options']);
  $variables['xml_link'] = l($xml_url, $variables['xml']['path'], $variables['xml']['options']);
  // Get image count.
  $variables['image_count'] = count($variables['gallery_images']);
  // Get the gallery option list in a displayable format.
  $variables['option_list'] = array();
  foreach ($variables['gallery_options'] as $option_key => $option_value) {
    $variables['option_list'][] = $option_key . '=' . $option_value;
  }
  // Get the library info in a displayable format.
  $library = juicebox_library_detect(FALSE, TRUE);
  $variables['lib_status'] = t('Locally Installed');
  if (empty($library['installed'])) {
    $variables['lib_status'] = t('Not Locally Installed');
  }
  if (!empty($library['version'])) {
    $variables['lib_status'] = $variables['lib_status'] . ', ' . $library['version'];
  }
}


/**
 * Returns HTML for the alternative "debug" display of a Juicebox gallery.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
 */
function theme_juicebox_debug_markup(&$variables) {
  $settings = $variables['settings'];
  $output = '';
  $custom_classes = '';
  if (!empty($settings['custom_parent_classes'])) {
    $custom_classes = ' ' . check_plain(trim($settings['custom_parent_classes']));
  }
  // Display a structured list of each debug detail.
  $output .= '<div class="juicebox-parent' . $custom_classes . '">' . "\n";
  // Add toggle links.
  $output .= theme('juicebox_toggle_embed_debug_markup', array('gallery_id' => $variables['gallery_id'], 'settings' => $settings, 'debug' => TRUE));
  $output .= '<div class="juicebox-container juicebox-placeholder juicebox-debug-info">' . "\n";
  $output .= '<p>' . t('Showing debugging information in place of gallery.') . '</p>';
  $output .= '<dl>';
  $output .= '<dt>' . t('Gallery ID') . '</dt>';
  $output .= '<dd>' . $variables['gallery_id'] . '</dd>';
  $output .= '<dt>' . t('XML URL') . '</dt>';
  $output .= '<dd>' . $variables['xml_link'] . '</dd>';
  $output .= '<dt>' . t('Juicebox Javascript Library') . '</dt>';
  $output .= '<dd>' . $variables['lib_status'] . '</dd>';
  $output .= '<dt>' . t('Image Count') . '</dt>';
  $output .= '<dd>' . $variables['image_count'] . ' ' . t('images') . '</dd>';
  $output .= '<dt>' . t('Juicebox Gallery Options') . '</dt>';
  $output .= '<dd>' . theme('item_list', array('items' => $variables['option_list'])) . '</dd>';
  $output .= '</dl>';
  $output .= '</div></div>';
  return $output;
}


/**
 * Proprocess logic for the juicebox_toggle_markup theme function.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
 */
function juicebox_preprocess_juicebox_toggle_embed_debug_markup(&$variables) {
  $settings = $variables['settings'];
  // Build the links needed for the toggles based on which view is active.
  $query = drupal_get_query_parameters();
  if (!empty($variables['debug'])) {
    unset($query['jb-debug']);
    $variables['gallery_link'] = l(t('gallery'), current_path(), array('query' => $query, 'fragment' => $variables['gallery_id']));
    $variables['info_link'] = t('debug info');
  }
  else {
    $query['jb-debug'] = 1;
    $variables['info_link'] = l(t('debug info'), current_path(), array('query' => $query, 'fragment' => 'debug-' . $variables['gallery_id']));
    $variables['gallery_link'] = t('gallery');
  }
}


/**
 * Returns HTML for a set of links that toggle between the normal and "debug"
 * display of a Juicebox gallery.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
 */
function theme_juicebox_toggle_embed_debug_markup(&$variables) {
  $settings = $variables['settings'];
  $output = '';
  if (!empty($settings['show_debug_links'])) {
    $output .= '<div id="debug-' . $variables['gallery_id'] . '">' . $variables['gallery_link'] . ' | ' . $variables['info_link'] . '</div>';
  }
  return $output;
}
