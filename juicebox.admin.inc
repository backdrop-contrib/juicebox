<?php


/**
 * @file
 * Administrative page callbacks for the juicebox module.
 */


/**
 * Menu callback: global configuration options to control Juicebox behaviour.
 *
 * @see juicebox_menu()
 */
function juicebox_admin_settings() {
  $library = juicebox_library_detect(TRUE, TRUE);
  $version = !empty($library['version']) ? $library['version'] : t('Unknown');
  $form['juicebox_admin_intro'] = array(
    '#markup' => t("These are global options for Juicebox administration and debugging purposes. Note that most Juicebox configuration options are set within each gallery's unique configuration form and not applied on a global scope like the values here. Please be sure to clear your site's caches via the !performance settings after making any changes here.", array('!performance' => l(t('performance'), 'admin/config/development/performance'))),
  );
  $form['juicebox_js_scope'] = array(
    '#type' => 'select',
    '#title' => t('Javscript Scope'),
    '#options' => array('header' => t('Header'), 'footer' => t('Footer')),
    '#default_value' => variable_get('juicebox_js_scope', 'header'),
    '#description' => t('The scope in which to add the Juicebox javascript library and all gallery-specific definitions.'),
  );
  $form['juicebox_apply_markup_filter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Filter all title and caption output for compatibility with Juicebox javascript (recommended)'),
    '#default_value' => variable_get('juicebox_apply_markup_filter', TRUE),
    '#description' => t('This option helps ensure title/caption output is syntactically compatible with the Juicebox javascript library by removing block-level tags.'),
  );
  $form['juicebox_show_debug_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show links to debug info'),
    '#default_value' => variable_get('juicebox_show_debug_links', FALSE),
    '#description' => t('Show links above each gallery to toggle the display of gallery debugging information.'),
  );
  $form['juicebox_enable_cors'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow galleries to be embedded remotely (CORS support)'),
    '#default_value' => variable_get('juicebox_enable_cors', FALSE),
    '#description' => t('Enable cross-origin resource sharing (CORS) for all generated Juicebox XML. This will allow all origins/domains to use Juicebox XML requested from this site for embedding purposes.'),
  );
  $form['juicebox_multilingual'] = array(
    '#type' => 'fieldset',
    '#title' => t('Multilingual options'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['juicebox_multilingual']['juicebox_translate_interface'] = array(
    '#type' => 'checkbox',
    '#title' => t('Translate the Juicebox javascript interface'),
    '#default_value' => variable_get('juicebox_translate_interface', FALSE),
    '#description' => t('Send interface strings to the Juicebox javascript after passing them through the Drupal translation system.'),
  );
  $form['juicebox_multilingual']['juicebox_translate_base_languagelist'] = array(
    '#type' => 'textarea',
    '#title' => t('Base string for interface translation'),
    '#default_value' => variable_get('juicebox_translate_base_languagelist', $library['base_languagelist']),
    '#description' => t('The base <strong>English</strong> interface text that Drupal should attempt to translate and pass to the Juicebox javascript for display (using the "languageList" configuration option). This text will be treated as a <strong>single string</strong> by Drupal and must be translated with a tool such as the Locale module. Note that this base string value will rarely change, and any changes made to it will break existing translations.'),
    '#wysiwyg' => FALSE,
    '#states' => array(
      // Hide the settings when the translate option is disabled.
      'invisible' => array(
        ':input[name="juicebox_translate_interface"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['juicebox_multilingual']['juicebox_translate_base_languagelist_suggestion'] = array(
    '#type' => 'item',
    '#title' => t('Suggested base string for currently detected Juicebox version (@version)', array('@version' => $version)),
    '#description' => '<pre>' . $library['base_languagelist'] . '</pre>',
    '#states' => array(
      // Hide the settings when the translate option is disabled.
      'invisible' => array(
        ':input[name="juicebox_translate_interface"]' => array('checked' => FALSE),
      ),
    ),
  );
  return system_settings_form($form);
}
