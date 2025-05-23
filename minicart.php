<?php
$debug = false;
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}

$cart = $_Registry->LoadClass('cart');
$cart->LoadCart();
$vars = array();
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
		<link rel="stylesheet" type="text/css" href="styles/cart.styles.css" />
	    <link rel="stylesheet" href="../styles/mmd_new_.css">
	</head>
<body>

<?=$miniCart['item_count'] = empty($_Totals['totalQuantity']) ? "0" : $_Totals['totalQuantity'];?>

</body>
</html>
	