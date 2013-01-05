<?php

/**
 * @file juicebox-view.tpl.php
 */
?>
<!--START JUICEBOX EMBED-->
<script>
  new juicebox({
    configUrl : '<?php print $config_url_path ?>',
    containerId : 'juicebox-container',
    galleryWidth : '<?php print $style_options['width'] ?>',
    galleryHeight : '<?php print $style_options['height'] ?>'
  });

</script>
<div id="juicebox-container"></div>
<!--END JUICEBOX EMBED-->
