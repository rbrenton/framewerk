<?php
require_once(__DIR__.'/../../common.php');

// temporary until _dir -> _root mapping
define('PHP_INCLUDE_ROOT',    DEPLOY_ROOT.'/'.PHP_INCLUDE_DIR);
define('PHP_ENGINE_ROOT',     DEPLOY_ROOT.'/'.PHP_ENGINE_DIR);
