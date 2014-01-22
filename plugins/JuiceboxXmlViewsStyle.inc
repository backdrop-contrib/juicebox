<?php


/**
 * @file
 * Juicebox XML loader that's used to load (and build via loaded methods) the
 * XML associated with a Drupal views style formatter plugin.
 */


/**
 * Class to load and build the XML associated with a Drupal views style
 * formatter plugin.
 */
class JuiceboxXmlViewsStyle implements JuiceboxXmlInterface {

  // Base properties that reference source data.
  protected $viewName;
  protected $viewDisplay;
  protected $viewArgs;
  // Dynamic loaded data storage.
  protected $settings = array();
  protected $view;

  /**
   * Constructor.
   *
   * @param array $id_args
   *   An indexed array of arguments that describe this gallery (and make up its
   *   XML URL). This information uniquely identifies the gallery and contains
   *   all the descriptive data needed to load it.
   */
  public function __construct($id_args) {
    // We need at least 3 data sources to build a view-based gallery (the type
    // along with 2 identifiers for the view display).
    if (empty($id_args) || count($id_args) < 3) {
      throw new Exception(t('Cannot initiate view style Juicebox XML due to insufficient ID args.'));
    }
    // Set data sources as properties.
    $this->viewName = $id_args[1];
    $this->viewDisplay = $id_args[2];
    $this->viewArgs = array_slice($id_args, 3);
    $this->view = views_get_view($this->viewName);
    if (empty($this->view)) {
      throw new Exception(t('Cannot initiate view style Juicebox XML because view @view cannot be found.', array('@view' => $this->viewName)));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function access() {
    // Check access directly through view methods. This should work no matter
    // what state the view is in (executed or not, etc.).
    return $this->view->access($this->viewDisplay);
  }

  /**
   * Load the Drupal data needed to build the gallery.
   */
  protected function loadFormatterData() {
    // Execute the view. This is easiest to do by simply calling the preview
    // method (as it handles all needed sub-processes).
    $this->view->preview($this->viewDisplay, $this->viewArgs);
    // If everything went correctly, our style plugin will contain a Juicebox
    // object. The state of this object will depend on caching (it will be
    // empty if views caching bypassed explicit render() methods), but
    // confirming it exists ensures we are working with a compatible formatter.
    if (!isset($this->view->style_plugin->juicebox)) {
      throw new Exception(t('Cannot load the views style data needed for Juicebox XML.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getXml() {
    // (Lazy) Load the data needed to build the gallery.
    $this->loadFormatterData();
    // See if if the galley is actually built. The loadFormatterData() method
    // typically triggers the init and build processes already, but not always
    // depending on view caching.
    if (!$this->view->style_plugin->juicebox->isInitialized()) {
      // We can force the galley to be initialized and built via the formatter's
      // render method.
      $this->view->style_plugin->render();
    }
    // Render the XML.
    return $this->view->style_plugin->juicebox->renderXml();
  }

}