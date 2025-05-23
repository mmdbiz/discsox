<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "utilities";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(false,true);

// for big uploads
@set_time_limit(120);

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

chdir("../");

$uploadStatus = array();
$uploadError = NULL;
$doImport = false;
$importResult = NULL;

// do import
if(isset($_REQUEST['import']) && count($_FILES) > 0){

	$import = $_Registry->LoadClass("import");
	
	$uploadStatus = $import->doUploads();

//$_Common->debugPrint($uploadStatus);

	
	foreach($uploadStatus as $fName=>$flds){
	
		if(isset($flds['ERROR'])){
			$uploadError .= $flds['ERROR'] . "<br />";
		}
		elseif(isset($flds['status'])){
			
			// load in database
			if(strtolower($fName) == "products.dat"){
				$importResult .= "<br /><br />" . $import->importProducts();
			}
			if(strtolower($fName) == "options.dat"){
				$importResult .= "<br /><br />" . $import->importOptions();
			}
		}
	}
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
	<head>
		<title>Import Data</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" >
		<script language="JavaScript">
			var sHeight = screen.height;
			var sWidth = screen.width;
			var styles = "admin.800.css";
			if(sWidth > 800){
				styles = "admin.1024.css";
			}
			if(sWidth > 1024){
				styles = "admin.1152.css";
			}
			if(sWidth > 1100){
				styles = "admin.1280.css";
			}
			document.write('<link rel="stylesheet" href="../stylesheets/' + styles + '" type="text/css">');
		</script>
		<style>
			td{
				vertical-align:middle;
			}
		</style>
	</head>
	<body style="margin-left:10px;margin-top:30px;">

		<?php if($importResult):?>

			<h4 align="center"><?=$importResult;?></h4>

		<?php elseif($uploadError):?>
		
			<h4 align="center"><?=$uploadError;?></h4>
		
		<?php endif;?>
	
		<form name="import" method="post" action="import.212.php" enctype="multipart/form-data">
		
			<table border="0" cellpadding="3" cellspacing="0" align="center" width="600">
				<tr><th align="left" colspan="2">Import QuikStore 2.12 Database Files</th></tr>
				<tr><td colspan="2">&nbsp;</td></tr>
				<tr>
					<td align="right">Upload products.dat: </td>
					<td><input name="products_dat" type="file" size="40"></td>
				</tr>
				<tr>
					<td align="right">Overwrite existing Products: </td>
					<td>
						<input type="radio" name="overwrite_products" value="true"> Yes &nbsp;
						<input type="radio" name="overwrite_products" value="false" checked> No
				</tr>
				<tr><td colspan="2">&nbsp;</td></tr>
				<tr>
					<td align="right">Upload options.dat: </td>
					<td><input name="options_dat" type="file" size="40"></td>
				</tr>
				<tr>
					<td align="right">Overwrite existing Options: </td>
					<td>
						<input type="radio" name="overwrite_options" value="true"> Yes &nbsp;
						<input type="radio" name="overwrite_options" value="false" checked> No
				</tr>
				<tr><td colspan="2">&nbsp;</td></tr>
				<tr><td colspan="2" align="center"><input type="submit" name="import" value="Upload/Import files"></td></tr>
			</table>
		
		</form>

	
	</body>
</html>
















