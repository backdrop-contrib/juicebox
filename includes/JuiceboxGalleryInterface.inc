<?php


/**
 * @file
 * Interface definition for the JuiceboxGallery class.
 */


/**
 * Interface definition for the Juicebox gallery class.
 */
interface JuiceboxGalleryInterface {

  public function addImage($src_data = array(), $title = '', $caption = '', $filter = TRUE);
  public function addOption($option_name, $option_value, $override = TRUE);
  public function getXml();
  public function getEmbed($custom_classes = '');
  public function getJavascript($xml_path);
  public function getChecksum();

}
