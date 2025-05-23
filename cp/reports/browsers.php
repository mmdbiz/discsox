<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

// Simple browser stats

$debug = false;
$sql = "SELECT browser,COUNT(browser) as total FROM orders GROUP BY browser ORDER BY total DESC";
$data = $_DB->getRecords($sql);
	

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Order Report</title>
<script	LANGUAGE="JavaScript">
//<!--
sWidth = screen.width;
var	styles = "admin.800.css";
if(sWidth >	850){
	styles = "admin.1024.css";
}
if(sWidth >	1024){
	styles = "admin.1152.css";
}
if(sWidth >	1100){
	styles = "admin.1280.css";
}
document.write('<link rel="stylesheet" href="../stylesheets/' + styles	+ '" type="text/css">');
//-->
</script>
</head>
<body class="mainBody">

<div align=center valign=top>

<?php if(count($data) == 0):?>

	<p><br />Cannot display browser counts. There are no orders in the system yet.</p>

<?php else:?>

	<h4><br />Order Counts by Browser Type</h4>

	<table border="0" cellpadding="3" cellspacing="0" width="300">
		<tr>
			<th class="cartHeader" nowrap>Browser</th>
			<th class="cartHeaderEnd" nowrap>Total Orders</th>
		</tr>
		<?php foreach($data as $i=>$fields):?>
		<tr>
			<td class="cartRow" align="left"><?=$fields['browser'];?></td>
			<td class="cartRowEnd"><?=$fields['total'];?></td>
		</tr>
		<?php endforeach;?>
	</table>

<?php endif;?>
	
</div>
<p>&nbsp;</p>
</body>
</html>
				
				
				
				
				