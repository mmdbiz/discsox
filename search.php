<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// get the login class and see if required
$login = $_Registry->LoadClass('login');
$login->checkLogin();

// load the cart so we can display mini-cart
$cart = $_Registry->LoadClass('cart');

// category lists
$parents = array();
$subcats = array();
include_once("cp/include/categories.inc");
$catClass = new Categories();
$catClass->listCategories(true);

$sql = "SELECT mid,mfg_name FROM `manufacturers` WHERE `display_mfg` = 'true' ORDER BY mfg_name";
$mfgSelectBox = $_Common->makeSelectFromQuery('mid','mfg_name','mid',$sql);

// get the search.html page
$vars = array('parents'=>$parents,'subcats'=>$subcats,'mfgSelectBox'=>$mfgSelectBox);
$page = $_Template->Open("templates/search.html",$vars,true);
?>
<?=$page;?>