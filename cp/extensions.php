<?php
$_isAdmin = true;
$_adminFunction = "extensions";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

if(count($_REQUEST) == 0){
	$_Common->redirect("config.welcome.php");
	exit;
}

$ext = $_Registry->LoadClass("extensions");

$phpFile = "extensions.php";
$idFld = "extid";
$fldProperties = array();
$data = array();

$add = false;
$edit = false;

foreach(array_keys($_REQUEST) as $i=>$key){
	$RUN = false;
	switch($key){
		case "add":
			$ext->add();
			$add = true;
			$RUN = true;
			break;
		case "edit":
			$ext->display(true);
			$edit = true;
			$RUN = true;
			break;
		case "modify":
			$ext->update();
			$ext->display(true);
			$edit = true;
			$RUN = true;
			break;
		case "insert":
			$ext->insert();
			$ext->display(true);
			$edit = true;
			$RUN = true;
			break;
		case "delete":
			$ext->delete();
			$ext->display();
			$RUN = true;
			break;
	} # End switch
	
	if($RUN){
		break;
	}
}

if(!$RUN){
	$ext->display();
}
$help = $ext->help;

?>


<html>
<head>
<meta name=vs_targetSchema content="http://schemas.microsoft.com/intellisense/ie5">
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="expires" content="0">
<title>Extensions</title>
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
document.write('<link rel="stylesheet" href="stylesheets/' + styles	+ '" type="text/css">');
function populateTextBoxes(form, IdField){
	 form.submit();
}
//-->
</script>

<style>
ul{
  font-size: 8pt;
  font-family: Verdana;
  color: #000064;
}
.itemHead{
	font-size: 14px;
}
</style>
</head>
<body>

<form method="POST" action="extensions.php">

<div style="text-align:	center">
<table border="0" width="90%" cellspacing="1" cellpadding="3">

	<?php if(!empty($help['section_help'])):?>
		<tr>
            <td align="left" colspan="2"><?=$help['section_help'];?></td>
        </tr>
		<tr>
            <td colspan="2">&nbsp;</td>
        </tr>
	<?php endif;?>


<?php if($add):?>

	<tr>
        <td align="left" colspan="2"><h4>Add New Extension</h4></td>
    </tr>

    <!-- Add screen -->
    <?php foreach($fldProperties as $key=>$props):?>
		<?php
		if($key == $idFld){
			continue;
		}
		$displayKey = ucwords(preg_replace("|\_|"," ",$key));
		if($key == "related_payment_form"){
			$value = $relatedFormSelect;
		}
		else{
			$value = $_DB->getDefaultValues($key);
		}
		?>
        <tr>
            <td align=right><?=$displayKey;?>: </td>
            <td>
				<?php if(substr($value,0,7) == "<select"):?>
					<?=$value;?>
				<?php else:?>
					<input type="text" name="<?=$key;?>" value="<?=$value;?>" size="50">
				<?php endif;?>
            </td>
        </tr>
    <?php endforeach;?>
        <tr>
            <td colspan="2">&nbsp;</td>
        </tr>
        <tr>
            <td colspan="2" align="center"><input type="submit" name="insert" value="Add Extension"></td>
        </tr>

<?php elseif($edit):?>

	<tr>
        <td align="left" colspan="2"><h4>Editing <?=ucwords(strtolower($data[0]['class_to_extend']));?> Extension</h4></td>
    </tr>


    <!-- Modify screen -->
    <input type="hidden" name="<?=$idFld;?>" value="<?=$data[0][$idFld];?>">
    <?php foreach($data[0] as $key=>$value):?>
		<?php
		if($key == $idFld){continue;}
		$displayKey = ucwords(preg_replace("|\_|"," ",$key));
		?>
		
		<?php if(!empty($help[$key])):?>
		<tr>
            <td align="center" colspan="2"><?=$help[$key];?></td>
        </tr>
		<?php endif;?>
		
        <tr>
            <td align=right><?=$displayKey;?>: </td>
            <td>
			<?php if(strtolower($value) == "true" || strtolower($value) == "false"):?>
				<?php if($value == "true"):?>
					<select name="<?=$key;?>">
						<option value="true" selected>True</option>
						<option value="false">False</option>
					</select>
				<?php else:?>
					<select name="<?=$key;?>">
						<option value="true">True</option>
						<option value="false" selected>False</option>
					</select>
				<?php endif;?>
			<?php else:?>
				<input type="text" name="<?=$key;?>" value="<?=$value;?>" size="60">
            <?php endif;?>
            </td>
        </tr>
    <?php endforeach;?>
        <tr>
            <td colspan="2">&nbsp;</td>
        </tr>
        <tr>
            <td colspan="2" align="center"><input type="submit" name="modify" value="Update Extension"></td>
        </tr>
<?php else:?>

	<?php if(count($data) > 0):?>

		<tr>
			<td align="left" colspan="3"><h4>Extensions</h4></td>
		</tr>
	
		<!-- List screen -->
		<?php $headers = array_keys($data[0]);?>
		<tr>
			<?php foreach($headers as $i=>$key):?>
			<?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
			<?php if($key == $idFld){continue;}?>
				<th align="left"><?=$displayKey;?></th>
			<?php endforeach;?>
			<th colspan="2">Manage</th>
		</tr>

		<?php
		$color = array();
		$color[0] = "#FFFFFF";
		$color[~0] = "#e2eDe2";
		$ck = 0;
		?>

		<?php foreach($data as $index=>$fields):?>
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<?php foreach($fields as $key=>$value):?>
					<?php if($key == $idFld){$id = $value; continue;}?>
					<td><?=$value;?></td>
				<?php endforeach;?>
				<td align=center valign=top><a href="<?=$phpFile;?>?edit=true&<?=$idFld;?>=<?=$id;?>">Edit</a></td>
				<td align=center valign=top><a href="<?=$phpFile;?>?delete=true&<?=$idFld;?>=<?=$id;?>">Delete</a></td>
			</tr>
		<?php endforeach;?>


	<?php else:?>
	
		<tr>
			<td align="left" colspan="3"><h4>No extensions have been defined.</h4></td>
		</tr>

	<?php endif;?>

<?php endif;?>

</table>
</div>
<p>&nbsp;</p>
</form>
</body>
</html>

