<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$categories = array();
$records = array();

$sql = "SELECT products.pid,products.name, product_categories.*, categories.category_name
		FROM products,product_categories,categories
		WHERE product_categories.pid = products.pid
		AND categories.catid = product_categories.catid";

$rs = $_DB->execute($sql);
while($row = $_DB->fetchrow($rs, "ASSOC")){
	$categories[$row['catid']] = $row['category_name'];
	$records[$row['catid']][] = array($row['pid'],$row['name']);
}
//$_Common->debugPrint($categories);


$XMLmap = '<?xml version="1.0" encoding="UTF-8"?>' . $_CR;
$XMLmap .= '<urlset xmlns="http://www.google.com/schemas/sitemap/0.84">' . $_CR;
$today = date("Y-m-d");
$linkCount = 0;

foreach($categories as $catid=>$desc){

	// category page
	$linkCount++;
	$XMLmap .= "<url>$_CR";
    $XMLmap .= "<loc>$website_url/products.php?catid=$catid&amp;category=" . urlencode($desc) . "</loc>$_CR";
    $XMLmap .= "<lastmod>$today</lastmod>$_CR";
    $XMLmap .= "<changefreq>monthly</changefreq>$_CR";
    $XMLmap .= "</url>$_CR";

	// pages in the category
	foreach($records[$catid] as $index=>$flds){
		
		$pid = urlencode($flds[0]);
		$name = urlencode($flds[1]);
		
		$linkCount++;
		$XMLmap .= "<url>$_CR";
		$XMLmap .= "<loc>$website_url/products.php?pid=$pid&amp;detail=true&amp;desc=$name</loc>$_CR";
		$XMLmap .= "<lastmod>$today</lastmod>$_CR";
		$XMLmap .= "<changefreq>daily</changefreq>$_CR";
		$XMLmap .= "</url>$_CR";
	}
}
$XMLmap .= "</urlset>$_CR";

$xmlExists = false;
if(file_exists("../../sitemap.xml")){
	$xmlExists = true;	
}

include_once("../include/ftp.inc");
$ftp = new Ftp();
$ftpRoot = $_CF['ftp']['document_root'];
$ftp->ChDir($ftpRoot);
$ftp->writeFile("sitemap.xml",$XMLmap);
$ftp->Chmod("sitemap.xml",755);
$ftp->Close();

?>
<html>
<head>
<title>Customer Report</title>
<script LANGUAGE="JavaScript">
//<!--
sWidth = screen.width;
var styles = "admin.800.css";
if(sWidth > 850){
    styles = "admin.1024.css";
}
if(sWidth > 1024){
    styles = "admin.1152.css";
}
if(sWidth > 1100){
    styles = "admin.1280.css";
}
document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');
//-->
</script>
</head>
<body class="mainBody">
	<div align="center" style="padding-top:25px;">
		<?php if($xmlExists):?>
			<p>The <a href="<?=$website_url;?>/sitemap.xml" target="new">site map</a> xml file has been updated.</p>
		<?php else:?>
			<p>The <a href="<?=$website_url;?>/sitemap.xml" target="new">site map</a> xml file has been created.</p>
		<?php endif;?>
	</div>
</body>
</html>






