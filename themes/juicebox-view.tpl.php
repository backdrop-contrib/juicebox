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
    galleryWidth : '<?php print check_plain($style_options['width']) ?>',
    galleryHeight : '<?php print check_plain($style_options['height']) ?>'
  });
</script>
<div id="juicebox-container"></div>
<!--END JUICEBOX EMBED-->
