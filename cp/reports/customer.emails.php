<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$data = $_DB->getRecords("SELECT DISTINCT billaddress_email, billaddress_firstname, billaddress_lastname
						  FROM customers 
						  WHERE billaddress_email != ''
						  AND email_list = 'true'
						  GROUP BY billaddress_email
						  ORDER BY billaddress_firstname, billaddress_lastname");

?>
<html>
<body>
<pre>
<?php if(count($data) > 0):?>
Name,E-mail Address
<?php foreach($data as $i=>$row):?>
<?=$row['billaddress_firstname'];?> <?=$row['billaddress_lastname'];?>,<?=$row['billaddress_email'];?><?=$_CR;?>
<?php endforeach;?>
<?php else:?>
	There is no data to display
<?php endif;?>
</pre>
</body>
</html>