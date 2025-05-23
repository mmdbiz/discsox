<?php
// initialize the program and read the config
include_once("include/initialize_simple.inc");
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
//$page = $_Template->Open("templates/search.html",$vars,true);
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Shipping Quote</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
<link rel="stylesheet" type="text/css" href="styles/cart.styles.css" />
</head>
<body>
		<?php foreach($records as $i=>$fields):?>
        
<p>sku:&nbsp;<?=$fields['sku'];?></p>
		<?php endforeach;?>
<p>&nbsp;</p>
</body>
</html>
