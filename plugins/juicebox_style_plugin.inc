<?php

/**
 * @file
 * Contains the Juicebox style plugin.
 */

/**
 * Style plugin to render each item in an ordered or unordered list.
 *
 * @ingroup views_style_plugins
 */
class juicebox_style_plugin extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();
    $options['width'] = '100%';
    $options['height'] = '100%';
    $options['title_field'] = array('default' => NULL);
    $options['image_field'] = array('default' => NULL);
    $options['thumb_field'] = array('default' => NULL);
    $options['caption_field'] = array('default' => NULL);
    $options['advanced'] = NULL;
    $options['advanced']['config'] = '';   
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    // Get the active field options
    $field_options = array('' => t('- None -'));
    $field_labels = $this->display->handler->get_field_labels();
    $field_options += $field_labels;

    $form['description'] = array(
      '#type' => 'markup',
      '#value' => '<div class="messages">' . t('Options for the Juicebox view.') . '</div>',
    );
    $form['width'] = array(
      '#type' => 'textfield',
      '#title' => t('Gallery Width'),
      '#default_value' => $this->options['width'],
      '#description' => t('Set the gallery width'),
    );
    $form['height'] = array(
      '#type' => 'textfield',
      '#title' => t('Gallery Heigth'),
      '#default_value' => $this->options['height'],
      '#description' => t('Set the gallery height'),
    );

    $form['image_field'] = array(
      '#type' => 'select',
      '#title' => t('Image Field'),
      '#default_value' => $this->options['image_field'],
      '#description' => t('Select the field from the view that should be used for each full-sized image in the Juicebox gallery. This will typically be an "image" field but any field that generates an "img" tag should work.'),
      '#options' => $field_options,
    );
    $form['thumb_field'] = array(
      '#type' => 'select',
      '#title' => t('Thumb Field'),
      '#default_value' => $this->options['thumb_field'],
      '#description' => t('Select the field from the view that should be used for each thumbnail in the Juicebox gallery. This will typically be an "image" field but any field that generates an "img" tag should work. You may want to add the field that you used for the "Image Field" <em>twice</em> to your view, and simple use a different (smaller) style formatter for the second one, and then select that field here.'),
      '#options' => $field_options,
    );
    $form['title_field'] = array(
      '#type' => 'select',
      '#title' => t('Title Field'),
      '#default_value' => $this->options['title_field'],
      '#description' => t('Select the field from the view that should be used for the title of each image in the Juicebox gallery.'),
      '#options' => $field_options,
    );
    $form['caption_field'] = array(
      '#type' => 'select',
      '#title' => t('Caption Field'),
      '#default_value' => $this->options['caption_field'],
      '#description' => t('Select the field from the view that should be used for the title of each image in the Juicebox gallery.'),
      '#options' => $field_options,
    );
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced Options'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['advanced']['config'] = array(
      '#type' => 'textarea',
      '#title' => t('Manual configuraton options'),
      '#default_value' => $this->options['advanced']['config'],
      '#description' => t('Define any general configuration options that should be used in this gallery (see: http://www.juicebox.net/support/config_options/). Add one per line in the format <strong>optionName = "optionValue"</strong>. These will be inserted <em>as is</em> within the configuration XML used in this gallery so take care to ensure your formatting is correct.'),
    );

  }

  function validate() {
    $errors = parent::validate();
    if ($this->display->handler->use_pager()) {
      $errors[] = t('The Juicebox style cannot be used with a pager. Disable the "Use pager" option for this display.');
    }
    return $errors;
  }

}
