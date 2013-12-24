<?php


/**
 * @file
 * A php-only set of methods to create the script and markup components of a
 * Juicebox gallery.
 */


class JuiceboxGallery implements JuiceboxGalleryInterface {

  /**
   * Base properies to contain structured gallery data.
   */
  public $id = '';
  public $options = array();
  public $images = array();
  
  /**
   * Constructor
   * 
   * @param string $id
   *   A unique string id that can represnet this gallery.
   */
  function __construct($id) {
    $this->id = $id;
  }
  
  /**
   * Add a new image to the gallery.
   * 
   * @param array $src_data
   *   Associative array containing all the source data for a gallery image,
   *   including the:
   *   - image_url: URL to the full image to display.
   *   - thumb_url: URL to the thumbnail to display for the image.
   *   - link_url: The Juicebox "link URL" value for the image.
   *   - link_target: The browser target value to use when following a link URL.
   * @param string $title
   *   The title markup to display for the image.
   * @param string $caption
   *   The caption markup to display for the image.
   * @param boolean $filter
   *   Whether-or-not to filter the $title and $caption values for syntactic
   *   compatibility with Juicebox.
   * @return boolean
   *   Returns TRUE on successful addition and FALSE on failure.
   */
  function addImage($src_data = array(), $title = '', $caption = '', $filter = TRUE) {
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
    // Add image to gallery.
    $this->images[] = array(
      'src_data' => $src_data,
      'title' => $title,
      'caption' => $caption,
    );
    return TRUE;
  }
  
  /**
   * Add a new Juicebox configuration option to the gallery.
   * 
   * @param string $option_name
   *   The option name/key to add.
   * @param string $option_value
   *   The option value to add.
   * @param boolean $override
   *   Whether-or-not to override any values that already exist in the set
   *   options for the passed name/key.
   * @return boolean
   *   Returns TRUE on successful addition and FALSE on failure.
   */
  function addOption($option_name, $option_value, $override = TRUE) {
    if (!$override && array_key_exists(strtolower($option_name), $this->options)) {
      return FALSE;
    }
    // Add option with lowercase key (allowing for future lookups).
    $this->options[strtolower($option_name)] = $option_value;
    return TRUE;
  }
  
  /**
   * Get the XML for a Juicebox gallery once images and options have been
   * added.
   * 
   * @return string
   *   Structured Juicebox XML describing a gallery. Note that no header data
   *   is set or returned here.
   */
  function getXml() {
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
      $juicebox_image->setAttribute('imageURL', $image['src_data']['image_url']);
      $juicebox_image->setAttribute('thumbURL', $image['src_data']['thumb_url']);
      $juicebox_image->setAttribute('linkURL', $image['src_data']['link_url']);
      $juicebox_image->setAttribute('linkTarget', $image['src_data']['link_target']);
      $juicebox_image_title = $juicebox_image->appendChild($dom->createElement('title'));
      $juicebox_image_title->appendChild($dom->createCDATASection($image['title']));
      $juicebox_image_caption = $juicebox_image->appendChild($dom->createElement('caption'));
      $juicebox_image_caption->appendChild($dom->createCDATASection($image['caption']));
    }
    return $dom->saveXML();
  }
  
  /**
   * Get the embed code for a Juicebox gallery once images and options have been
   * added.
   * 
   * @param string $custom_classes
   *   Any custom classes to include as part of the Juicebox parent container.
   * @return string
   *   Embed code markup describing a gallery.
   */
  function getEmbed($custom_classes = '') {
    $output = '';
    $output .= '<div class="juicebox-parent' . $custom_classes . '">';
    $output .= '<div id="' . $this->id . '" class="juicebox-container">';
    $output .= '</div></div>';
    return $output;
  }
  
  /**
   * Get the javascript code for a Juicebox gallery once images and options have
   * been added.
   * 
   * @param string $xml_path
   *   The path to the Juicebox XML for this gallery.
   * @return string
   *   Javascript code describing a gallery. Note that <script> tags are
   *   excluded.
   */
  function getJavascript($xml_path) {
    $output = '';
    $output .= "new juicebox({";
    $output .= "configUrl:'" . $xml_path . "',";
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
   * Get a gallery checksum.
   * 
   * @return string
   *   An md5 hash of the image and option data that can be used to validate
   *   the uniqueness of this gallery's data and settings.
   */
  function getChecksum() {
    return md5(json_encode($this->images) . json_encode($this->options));
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
