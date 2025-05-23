<?php
$_isAdmin = true;
$_adminFunction = "gateways";

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

$gateway = $_Registry->LoadClass("Gateway");

$phpFile = "gateways.php";
$idFld = "pgid";
$fldProperties = array();
$data = array();

$add = false;
$edit = false;
$helpEditable = false;

ksort($_REQUEST);

foreach(array_keys($_REQUEST) as $i=>$key){
    $RUN = false;
    switch($key){
        case "add":
            $gateway->add();
            $add = true;
            $RUN = true;
            break;
        case "apply":
            $gateway->applyUpdate();
            $gateway->display(true);
            $RUN = true;
            break;
        case "delete":
            $gateway->delete();
            $gateway->display();
            $RUN = true;
            break;
        case "edit":
            $gateway->display(true);
            $edit = true;
            $RUN = true;
            break;
        case "modify":
            $gateway->update();
            $gateway->display(true);
            $edit = true;
            $RUN = true;
            break;
        case "upload":
            $gateway->Upload();
            $gateway->display(true);
            $RUN = true;
            break;
    } # End switch

    if($RUN){
        break;
    }
}

if(!$RUN){
	$gateway->display();
}

$relatedForms = explode(",",$_CF['payment_methods']['accepted_payment_methods']);
list($relatedFormSelect,$junk) = $_Common->makeSelectBox("related_payment_form",$relatedForms,$relatedForms,$_CF['payment_methods']['default_payment_method']);

$help = $gateway->help;
?>

<html>
<head>
<meta name=vs_targetSchema content="http://schemas.microsoft.com/intellisense/ie5">
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="expires" content="0">
<title>Payment Gateways</title>
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

<form method="POST" action="gateways.php" enctype="multipart/form-data">

<div style="text-align:	center">
<table border="0" width="70%" cellspacing="1" cellpadding="3">

<?php if($add):?>

	<!-- Add screen -->
	<tr>
        <td align="center" colspan="2"><h4>Upload Payment Gateway</h4></td>
    </tr>
    <tr>
        <td align="right" width="30%">Select file: </td>
        <td width="70%"><input name="upfile" type="file" size="40"></td>
    </tr>
    <tr>
        <td colspan="2">&nbsp;</td>
    </tr>
    <tr>
        <td colspan="2" align="center"><input type="submit" name="upload" value="Upload Gateway"></td>
    </tr>

<?php elseif($edit):?>

	<?php error_reporting(E_PARSE|E_WARNING);?>

	<tr>
        <td align="center" colspan="2"><h4><?=ucwords(strtolower($data[0]['gateway_name']));?> Payment Gateway</h4></td>
    </tr>

	<?php if(!empty($help['section_help']) || $helpEditable):?>
		<tr>
			<td>&nbsp;</td>
            <td align="left">
				<?php if($helpEditable):?>
					<textarea name="help[section_help]" rows="5" cols="70" wrap="virtual" ID="Textarea1"><?=stripslashes($help['section_help']);?></textarea>
				<?php else:?>
					<?=nl2br(stripslashes($help['section_help']));?>
				<?php endif;?>            
            </td>
        </tr>
		<tr>
            <td colspan="2">&nbsp;</td>
        </tr>
	<?php endif;?>


    <!-- Modify screen -->
    <input type="hidden" name="pgid" value="<?=$data[0]['pgid'];?>">
    <input type="hidden" name="gateway_name" value="<?=$data[0]['gateway_name'];?>">
    
    <?php foreach($data as $i=>$flds):?>
        <?php
			$key = $flds['key'];
			$value = $flds['value'];
        
			if($key == $idFld){continue;}
			$displayKey = ucwords(preg_replace("|\_|"," ",$key));
		?>
		
		<?php if(!empty($help[$key]) || $helpEditable):?>
		<tr>
			<td>&nbsp;</td>
            <td class="comments" align="left">
				<?php if($helpEditable):?>
					<textarea name="help[<?=$key;?>]" rows="5" cols="70" wrap="virtual" ID="Textarea2"><?=stripslashes($help[$key]);?></textarea>
				<?php else:?>
					<?=nl2br(stripslashes($help[$key]));?>
				<?php endif;?>
			</td>
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
            <td colspan="2" align="center"><input type="submit" name="modify" value="Update Gateway"></td>
        </tr>
<?php else:?>

	<tr>
        <td align="center" colspan="7"><h4>Payment Gateways</h4></td>
    </tr>

    <!-- List screen -->
    <?php $headers = array_keys($data[0]);?>
    <tr>
        <?php foreach($headers as $i=>$key):?>
        <?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
        <?php
			$align = "left";
			if($key == $idFld){
				continue;
			}
			if($key == "active" || $key == "debug"){
				$align = "center";
			}
		?>
            <th align="<?=$align;?>"><?=$displayKey;?></th>
        <?php endforeach;?>
        <th colspan="2">Manage</th>
    </tr>

    <?php
		$color = array();
		$color[0] = "#FFFFFF";
		$color[~0] = "#e2eDe2";
        $ck = 0;
        $cols = 2;
    ?>

    <?php foreach($data as $index=>$fields):?>
        <tr bgcolor="<?=$color[$ck = ~$ck];?>">
            <?php foreach($fields as $key=>$value):?>
                <?php if($key == $idFld){$id = $value; continue;}?>
                
                <?php
					$val = $value;
					$align = "left";
					if($value == 'true' || $value == 'false'){
						$fldName = $key . '[' . $id . ']';
						list($val,$junk) = $_Common->makeSelectBox($fldName,array('true','false'),array('true','false'),$value);
						$align = "center";
					}
					elseif($key == "related_payment_form"){
						$forms = array_keys($_DB->getRecords("SELECT method FROM payment_methods WHERE active = 'true'",'method'));
						$forms[] = 'none';
						$fldName = $key . '[' . $id . ']';
						list($val,$junk) = $_Common->makeSelectBox($fldName,$forms,$forms,$value);
					}
					$cols++;
				?>                
                
                <td align="<?=$align;?>"><?=$val;?></td>
            <?php endforeach;?>
            <td align=center valign=top><a href="<?=$phpFile;?>?edit=true&<?=$idFld;?>=<?=$id;?>">Edit</a></td>
            <td align=center valign=top><a href="<?=$phpFile;?>?delete=true&<?=$idFld;?>=<?=$id;?>">Delete</a></td>
        </tr>
    <?php endforeach;?>
		<tr>
            <td colspan="<?=$cols;?>" align="center">&nbsp;</td>
        </tr>
		<tr>
            <td colspan="<?=$cols;?>" align="center">
				<input type="submit" name="apply" value="Apply Changes">
			</td>
        </tr>
<?php endif;?>

</table>
</div>
<p>&nbsp;</p>
</form>
</body>
</html>