<?php


/**
 * @file
 * A php-only set of methods to create the script and markup components of a
 * Juicebox gallery.
 */


/**
 * Class to generate the script and markup for a Juicebox gallery.
 */
class JuiceboxGallery implements JuiceboxGalleryInterface {

  /**
   * Base properies to contain structured gallery data.
   */
  protected $id = '';
  protected $options = array();
  protected $images = array();

  /**
   * Constructor
   *
   * @param string $id
   *   A unique string id that can represnet this gallery.
   */
  public function __construct($id = '') {
    $this->id = $id;
  }

  /**
   * {@inheritdoc}
   */
  public function setId($value, $reset = TRUE) {
    if ($reset || empty($this->id)) {
      $this->id = $value;
      return TRUE;
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function getId() {
    return $this->id;
  }

  /**
   * {@inheritdoc}
   */
  public function addImage($src_data = array(), $title = '', $caption = '', $filter = TRUE, $override_id = NULL, $offset = NULL) {
    // If we are anticipating an override, but there is nothing to override,
    // don't do anything. Also, override_id and offset are mutually exclusive.
    if (isset($override_id) && (empty($this->images[$override_id]) || isset($offset))) {
      return FALSE;
    }
    // Make sure we have sufficient image data to work with.
    if (!isset($src_data['image_url']) || !isset($src_data['thumb_url'])) {
      return FALSE;
    }
    if (!isset($src_data['link_url'])) {
      $src_data['link_url'] = $src_data['image_url'];
    }
    if (empty($src_data['link_target'])) {
      $src_data['link_target'] = '_blank';
    }
    // Filter markup if needed.
    if ($filter) {
      $title = $this->filterMarkup($title);
      $caption = $this->filterMarkup($caption);
    }
    // Add image to gallery, overriding if necessary.
    $addition = array(
      'src_data' => $src_data,
      'title' => $title,
      'caption' => $caption,
    );
    if (isset($override_id)) {
      $this->images[$override_id] = $addition;
    }
    elseif (isset($offset)) {
      array_splice($this->images, $offset, 0, array($addition));
    }
    else {
      $this->images[] = $addition;
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function getImages() {
    return $this->images;
  }

  /**
   * {@inheritdoc}
   */
  public function removeImage($id) {
    if (!empty($this->images[$id])) {
      unset($this->images[$id]);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function addOption($option_name, $option_value, $override = TRUE) {
    // Always use lowercase keys to allow for future lookups.
    $option_name = strtolower($option_name);
    if (!empty($this->options[$option_name]) && !$override) {
      return FALSE;
    }
    // Add option,
    $this->options[$option_name] = $option_value;
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function getOptions() {
    return $this->options;
  }

  /**
   * {@inheritdoc}
   */
  public function removeOption($option_name) {
    $option_name = strtolower($option_name);
    if (!empty($this->options[$option_name])) {
      unset($this->options[$option_name]);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function getChecksum() {
    return md5(json_encode($this->images) . json_encode($this->options));
  }

  /**
   * {@inheritdoc}
   */
  public function renderXml() {
    // We use DOMDocument instead of a SimpleXMLElement to build the XML as it's
    // much more flexible (CDATA is supported, etc.).
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->formatOutput = TRUE;
    $juicebox = $dom->appendChild($dom->createElement('juicebox'));
    foreach ($this->options as $option => $value) {
      $juicebox->setAttribute($option, $value);
    }
    foreach ($this->images as $image) {
      $juicebox_image = $juicebox->appendChild($dom->createElement('image'));
      // Standard source data should always be available.
      $juicebox_image->setAttribute('imageURL', $image['src_data']['image_url']);
      $juicebox_image->setAttribute('thumbURL', $image['src_data']['thumb_url']);
      $juicebox_image->setAttribute('linkURL', $image['src_data']['link_url']);
      $juicebox_image->setAttribute('linkTarget', $image['src_data']['link_target']);
      // Multi-size source data may only be available in certain configurations.
      if (!empty($image['src_data']['image_url_small'])) {
        $juicebox_image->setAttribute('smallImageURL', $image['src_data']['image_url_small']);
      }
      if (!empty($image['src_data']['image_url_large'])) {
        $juicebox_image->setAttribute('largeImageURL', $image['src_data']['image_url_large']);
      }
      $juicebox_image_title = $juicebox_image->appendChild($dom->createElement('title'));
      $juicebox_image_title->appendChild($dom->createCDATASection($image['title']));
      $juicebox_image_caption = $juicebox_image->appendChild($dom->createElement('caption'));
      $juicebox_image_caption->appendChild($dom->createCDATASection($image['caption']));
    }
    return $dom->saveXML();
  }

  /**
   * {@inheritdoc}
   */
  public function renderEmbed() {
    $output = '';
    $output .= '<div class="juicebox-parent">';
    $output .= '<div id="' . $this->id . '" class="juicebox-container">';
    $output .= '</div></div>';
    return $output;
  }

  /**
   * {@inheritdoc}
   */
  public function renderJavascript($xml_url) {
    $output = '';
    $output .= "new juicebox({";
    $output .= "configUrl:'" . $xml_url . "',";
    $output .= "containerId:'" . $this->id . "'";
    // Add options that need to be loaded immediately (before XML is available).
    $load_before_xml = array('gallerywidth', 'galleryheight', 'backgroundcolor', 'themeurl', 'baseurl', 'showpreloader', 'debugmode');
    foreach ($load_before_xml as $key => $option) {
      if (!empty($this->options[$option])) {
        $output .= ',';
        $output .= $option . ":'" . $this->options[$option] . "'";
      }
    }
    $output .= "});";
    return $output;
  }

  /**
   * Filter markup for valid display in a Juicebox gallery.
   *
   * Some markup that validates fine via external filters will not be
   * syntactically valid once rendered within Juicebox. This is because Juicebox
   * will wrap titles and captions in block-level tags, like <p>, making any
   * block-level elements they contain invalid. This filter accommodates for
   * this and is meant to be applied AFTER any external filters.
   *
   * @param string $markup
   *   The markup to be filtered after it has been processed externally.
   * @return string
   *   Valid filtered markup ready for display in a Juicebox gallery.
   */
  protected function filterMarkup($markup) {
    // Set inline elements that are safe in a Juicebox gallery. References:
    // http://www.htmlhelp.com/reference/html40/inline.html
    // https://developer.mozilla.org/en-US/docs/Web/Guide/HTML/HTML5/HTML5_element_list
    $valid_elements = "<a><abbr><acronym><b><basefont><bdi><bdo><big><br><cite><code><data><del><dfn><em><font><i><img><ins><kbd><label><mark><q><rp><rt><ruby><s><samp><small><span><strike><strong><sub><sup><time><tt><u><var><wbr>";
    $markup = strip_tags($markup, $valid_elements);
    // Also remove newlines to keep the output concise.
    $markup = str_replace(array("\r", "\n"), '', $markup);
    return $markup;
  }

}
