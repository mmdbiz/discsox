<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

$error = NULL;
if(empty($_REQUEST['id'])){
	$error = "Unauthorized access";
}
else{
	if($_Registry->file_exists_incpath('downloads.inc')){
		$downloads = $_Registry->LoadClass('downloads');
		$downloads->getFile();
	}
	else{
		$error = "Unauthorized access";
	}
}
?>
<?php if($error):?>
	<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
	<html>
		<head>
			<title><?=$error;?></title>
			<link rel="stylesheet" type="text/css" href="styles/cart.styles.css">
		</head>
		<body>
			<p>&nbsp;</p>
			<p align="center"><?=$error;?></p>
		</body>
	</html>
<?php endif;?>
