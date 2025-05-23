<?php
$_isAdmin = true;
$_adminFunction = "users";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

if(count($_REQUEST) == 0){
    $_Common->redirect("welcome.php");
    exit;
}

$users = $_Registry->LoadClass("users");
$fldProperties = array();
$data = array();
$add = false;
$edit = false;
$idFld = "uid";
$type = "users";
$adminFunctions = array();

if(!empty($_REQUEST['type'])){
	$type = $_REQUEST['type'];
}
if($type == "customers"){
	$idFld = "custid";	
}

if($type != "customers" && $type != "users"){
	die("invalid user type selected");
}

if($type == "users"){
	$users->getFunctionList();
	$adminFunctions = $users->adminFunctions;
}

foreach(array_keys($_REQUEST) as $i=>$key){

    $RUN = false;
    switch($key){
        case "list":
            $users->display($type);
            $RUN = 1;
            break;
        case "add":
			$add = true;
            $users->add($type);
            $RUN = 1;
            break;
        case "edit":
			$edit = true;
            $users->display($type,true);
            $RUN = 1;
            break;
        case "update":
            $users->update($type);
            $users->display($type,true);
            $edit = true;
            $RUN = 1;
            break;
        case "delete":
            $users->update($type);
            $users->display($type);
            $RUN = 1;
            break;
    } // End switch

    if($RUN){
        break;
    }

}
    # If some other function was tried,
    # just show the home page.

if(!$RUN){
    $_Common->redirect("welcome.php");
}

?>

<html>
<head>
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="expires" content="0">
<title>Edit Users</title>
<script	LANGUAGE="JavaScript">
//<!--
if(!eval(parent.menu)) {
    top.parent.content.location.href = "welcome.php";
}
else{
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "user.menu.html"){
		parent.menu.location = 'menus/user.menu.html';
	}
}
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
  background-image:	url('images/gradient.jpg');
}
</style>
</head>
<body>
<form method="POST"	action="users.php">
<div align="center">
<table border="0" width="70%" cellspacing="1" cellpadding="3" align="center">

<?php if($add):?>

	<tr>
	<?php if($type == "customers"):?>
		<td colspan="2" align="center"><h4>Add Customer</h4></td>
		<?php $buttonText = "Add Customer";?>
	<?php else:?>
		<td colspan="2" align="center"><h4>Add User</h4></td></tr>
		<?php $buttonText = "Add User";?>
		<tr>
			<td colspan="2" align="left">
				<b>NOTE:</b> The rights below determine what functions an end user can run in the control panel.<br/><br/>
			</td>
	<?php endif;?>
	</tr>

    <!-- Add screen -->
    <?php foreach($fldProperties as $key=>$props):?>
        <?php if($key == $idFld || $key == 'cid'){continue;}?>
        <?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
        <?php $value = $_DB->getDefaultValues($key);?>
        <tr>
            <td valign="top" align="right" style="padding-top:6px;"><?=$displayKey;?>: </td>
            <?php if(strstr($value,"<select")):?>
				<td><?=$value;?></td>
			<?php elseif($key == "rights" && count($adminFunctions) > 0):?>
				<td valign="top">
					<table border="0" cellspacing="0" cellpadding="2">
					<?php foreach($adminFunctions as $aKey=>$aVal):?>
					<?php $label = str_replace("_"," ",$aKey); ?>
					<tr>
						<td><input type="checkbox" name="rights[]" value="<?=$aKey;?>"></td><td><?=ucwords($label);?></td>
					</tr>
					<?php endforeach;?>
					</table>
				</td>
            <?php else:?>
				<?php if($props[1] == "text"):?>
					<td><textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea></td>
				<?php else:?>
		            <td><input type="text" name="<?=$key;?>" value="<?=$value;?>" size="50"></td>
		        <?php endif;?>
	        <?php endif;?>
        </tr>
    <?php endforeach;?>

	<tr><td colspan="2"><hr size="1" noshade></td></tr>
	<tr>
		<td colspan="2" align="center">
			<input type="hidden" name="insert" value="true">
			<input type="hidden" name="type" value="<?=$type;?>">
			<input type="submit" name="update" value="<?=$buttonText;?>">
		</td>
	</tr>

<?php elseif($edit):?>

	<tr>
	<?php if($type == "customers"):?>
		<td colspan="2" align="center"><h4>Edit Customer</h4></td>
		<?php $buttonText = "Update Customer";?>
	<?php else:?>
		<td colspan="2" align="center"><h4>Edit User</h4></td></tr>
		<?php $buttonText = "Update User";?>
		<tr>
			<td colspan="2" align="left">
				<b>NOTE:</b> The rights below determine what functions an end user can run in the control panel.<br/><br/>
			</td>
	<?php endif;?>
	</tr>
	
    <!-- Modify screen -->
    <input type="hidden" name="<?=$idFld;?>" value="<?=$data[0][$idFld];?>">
    <input type="hidden" name="type" value="<?=$type;?>">
    <?php foreach($data[0] as $key=>$value):?>
		
		<?php
			if($key == $idFld || $key == 'cid'){
				continue;
			}
			$displayKey = ucwords(preg_replace("|\_|"," ",$key));
			$valign = "middle";
			if($displayKey == "Cust Notes"){
				$valign = "top";
			}
		?>
        
        <tr>
            <td valign="<?=$valign;?>" align="right">
				<?php if($_CF['login']['encrypt_password'] && $key == "password"):?>
					Encrypted <?=$displayKey;?>: 
				<?php else:?>
					<?=$displayKey;?>: 
				<?php endif;?>
			</td>

            <?php if($key != "rights" && strstr($value,"<select")):?>
				<td><?=$value;?></td>
				
			<?php elseif($key == "rights" && count($adminFunctions) > 0):?>
				<td valign="top">

					<table border="0" cellspacing="0" cellpadding="2">
					<?php foreach($adminFunctions as $aKey=>$aVal):?>
					<?php $label = str_replace("_"," ",$aKey); ?>
					<tr>
						<?php if(isset($value[$aKey])):?>
							<td><input type="checkbox" name="rights[]" value="<?=$aKey;?>" checked></td><td><?=ucwords($label);?></td>
						<?php else:?>
							<td><input type="checkbox" name="rights[]" value="<?=$aKey;?>"></td><td><?=ucwords($label);?></td>
						<?php endif;?>
					</tr>
					<?php endforeach;?>
					</table>

				</td>
				
            <?php else:?>
            
				<?php if($fldProperties[$key][1] == "text"):?>
					<td><textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea></td>
				<?php else:?>
		            <td><input type="text" name="<?=$key;?>" value="<?=$value;?>" size="50"></td>
		        <?php endif;?>

	        <?php endif;?>
        </tr>
    <?php endforeach;?>

	<tr><td colspan="2"><hr size="1" noshade></td></tr>
	<tr>
		<td colspan="2" align="center">
			<input type="submit" name="update" value="<?=$buttonText;?>">
		</td>
	</tr>


<?php else:?>

	<?php if(count($data) == 0):?>
	
		<tr><td align="center">No <?=$type;?> to display.</td></tr>
	
	<?php else:?>


		<?php $colspan = count(array_keys($data[0]));?>

		<tr>
		<?php if($type == "customers"):?>
			<td colspan="<?=$colspan;?>" align="center"><h4>Customer List</h4></td>
		<?php else:?>
			<td colspan="<?=$colspan;?>" align="center"><h4>User List</h4></td>
		<?php endif;?>
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
				<td align=center valign=top><a href="users.php?edit=true&<?=$idFld;?>=<?=$id;?>&type=<?=$type;?>">Edit</a></td>
				<td align=center valign=top><a href="users.php?delete=true&<?=$idFld;?>=<?=$id;?>&type=<?=$type;?>">Delete</a></td>
			</tr>
		<?php endforeach;?>
		
	<?php endif;?>

<?php endif;?>

</table>
</div>
</form>
</body>
</html>





