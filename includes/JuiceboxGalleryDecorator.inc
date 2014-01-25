<?php


/**
 * @file
 * Base decorator class for a Juicebox Gallery.
 *
 * We use a decorator pattern here (as opposed to just extending
 * JuiceboxGalleryInterface) so that multiple versions of
 * JuiceboxGalleryInterface classes can be supported simultaneously and
 * swapped-out dynamically. This may be needed to support different embed and
 * XML structures that may be required by different versions of the Juicebox
 * library.
 */


/**
 * Base decorator class for a Juicebox Gallery.
 */
abstract class JuiceboxGalleryDecorator implements JuiceboxGalleryInterface {

  // Base object to be decorated.
  protected $gallery;

  /**
   * Constructor.
   *
   * @param JuiceboxGalleryInterface $juicebox
   *   A plain-PHP Juicebox gallery object.
   */
  public function __construct(JuiceboxGalleryInterface $gallery) {
    $this->gallery = $gallery;
  }

  /**
   * {@inheritdoc}
   */
  public function setId($value, $reset = TRUE) {
    return $this->gallery->setId($value, $reset);
  }

  /**
   * {@inheritdoc}
   */
  public function getId() {
    return $this->gallery->getId();
  }

  /**
   * {@inheritdoc}
   */
  public function addImage($src_data = array(), $title = '', $caption = '', $filter = TRUE, $override_id = NULL) {
    // We should always get the filter setting from global variables.
    $filter = variable_get('juicebox_apply_markup_filter', TRUE);
    return $this->gallery->addImage($src_data, $title, $caption, $filter, $override_id);
  }

  /**
   * {@inheritdoc}
   */
  public function getImages() {
    return $this->gallery->getImages();
  }

  /**
   * {@inheritdoc}
   */
  public function removeImage($id) {
    return $this->gallery->removeImage($id);
  }

  /**
   * {@inheritdoc}
   */
  public function addOption($option_name, $option_value, $override = TRUE) {
    return $this->gallery->addOption($option_name, $option_value, $override);
  }

  /**
   * {@inheritdoc}
   */
  public function getOptions() {
    return $this->gallery->getOptions();
  }

  /**
   * {@inheritdoc}
   */
  public function removeOption($option_name) {
    return $this->gallery->removeOption($option_name);
  }

  /**
   * {@inheritdoc}
   */
  public function getChecksum() {
    return $this->gallery->getChecksum();
  }

  /**
   * {@inheritdoc}
   */
  public function renderXml() {
    return $this->gallery->renderXml();
  }

  /**
   * {@inheritdoc}
   */
  public function renderEmbed() {
    return $this->gallery->renderEmbed();
  }

  /**
   * {@inheritdoc}
   */
  public function renderJavascript($xml_path) {
    return $this->gallery->renderJavascript($xml_path);
  }

}
