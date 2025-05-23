<?php
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program, read the config(s) and set include paths
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

//$_Common->debugPrint($_REQUEST);

foreach($_REQUEST as $key=>$value){
	switch($key){

		case "update":
			doUpdates();
			break;

		case "delete":
			if(!empty($_REQUEST['dlid'])){
				$dlid = $_REQUEST['dlid'];
				$_DB->execute("DELETE FROM downloads WHERE `dlid` = '$dlid' LIMIT 1");	
			}
			break;
	}
}

$maxToDisplay = 25;
$hits = 0;
$start = 0;
$end = 0;
$count = 0;
$links = "";
$limit = null;
$searchFields = array();
$where = null;
$sql = "SELECT * FROM downloads";
$count = $_DB->getCount('downloads');

if($count > 1){
	$RS = $_DB->execute("SHOW FIELDS FROM downloads");
	while($row = $_DB->fetchrow($RS)){
		$fldName = strtolower($row[0]);
		$searchFields[$fldName] = 1;
	}
	unset($searchFields['dlid']);
	$_DB->free_result($RS);
	
	// check how many rows to display
	$max = $maxToDisplay;
	if(!empty($_REQUEST['max'])){
		if($max == "All" || $max > $count){
			$max = $count;
		}
		else{
			$max = intval($_REQUEST['max']);
		}
	}
	$maxToDisplay = $max;
	if(!empty($_REQUEST['hits'])){
		$hits = intval($_REQUEST['hits']);
	}
	if(!empty($_REQUEST['searchfield']) && !empty($_REQUEST['searchtext'])){
		foreach($searchFields as $fld=>$j){
			if($_REQUEST['searchfield'] == $fld){
				$str = strtoupper(trim($_REQUEST['searchtext']));
				$len = strlen($str);
				$where = "WHERE LEFT(UPPER($fld),$len) = '$str'";
				break;
			}
		}
	}
	if($where){
		$sql .= " $where";
	}
	
	list($start,$end,$limit) = $_DB->getLimits($count,$max,"downloads.php");
	$links = $_DB->previousNextLinks;

	if($limit){
		$sql .= $limit;
	}
//$_Common->debugPrint($searchFields,$sql);

	$data = $_DB->getRecords($sql);
}

$color = array();
$color[~0] = "#e2eDe2";
$color[0] = "#FFFFFF";
$ck = 0;

function doUpdates(){
	global $_Common,$_DB;
	if(!empty($_REQUEST['dlid'])){
		foreach($_REQUEST['dlid'] as $i=>$dlid){
			$sql = null;
			$fields = null;
			foreach($_REQUEST as $key=>$values){
				if(is_array($values) && $key != 'dlid'){
					
					if($values[$i] == ''){
						continue;
					}
					
					if($fields){
						$fields .= ", `$key` = '".$values[$i]."'";
					}
					else{
						$fields .= "`$key` = '".$values[$i]."'";
					}
				}
			}
			if($fields){
				$sql = "UPDATE downloads SET $fields WHERE `dlid` = '$dlid'";
				//$_Common->debugPrint($sql);
				$_DB->execute($sql);
			}
		}
	}
}
// creates the select box for max to display
function maxSelect(){
	
    global $_Common,$maxToDisplay;
    
    $names = array("10","25","50","100","All");
    
    $default = $maxToDisplay;
    if(!empty($_REQUEST['max'])){
		$default = $_REQUEST['max'];
		$_SESSION['max'] = $default;
	}
	elseif(!empty($_SESSION['max'])){
		$default = $_SESSION['max'];
	}
    list($select,$selected) = $_Common->makeSelectBox('max',$names,$names,$default,true);
    return $select;
}
?>

<html>
<head>
<title>Inventory List</title>
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
<body>
<h4 align="center">Downloads List</h4>
<form method="post" action="downloads.php">

	<table border="0" cellpadding="3" cellspacing="0" width="700" align="center">

	<tr bgcolor="#E8E8E8">
		<td align="left" colspan="2">
			<table border="0" cellpadding="3" cellspacing="0" width="100%">
				<tr>
					<td>
						<b>Search Customers:</b><br />
						(Partial words accepted)
					</td>
					<td align="right" style="padding-right:10px;">
						<table border="0" cellpadding="2" cellspacing="0" width="100%">
							<tr>
								<td align="right">Search Field:</td>
								<td>
									<?=$_Common->makeSimpleSelectBox('searchfield',array_keys($searchFields),array_keys($searchFields),null);?>
								</td>
								<td align="right">Search Text:</td>
								<td>
									<input type="text" name="searchtext" size="15">&nbsp;
									<input type="submit" name="search" value="Go" onClick="this.form.method = 'get'">
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<hr size="1" noshade>
		</td>
	</tr>
	<tr bgcolor="#E8E8E8">
		<td align="left">
			Display <?=maxSelect();?> per page
		</td>
		<td align="right">
			<?=$start . " - " . $end . " of " . $count . " records";?>				
		</td>
	</tr>
	<tr bgcolor="#E8E8E8">
		<td align="left">&nbsp;</td>
		<td align="right"><?=$links;?></td>
	</tr>
	<tr><td colspan="2">&nbsp;</td></tr>
	</table>





	<table border=1 cellpadding=3 cellspacing=0 align="center">
	<tr>
		<th align=left>order_number</th>
		<th align=left>email</th>
		<th align=left>filename</th>
		<th align=left>url</th>
		<th align=left>complete</th>
		<th align=left>expire_date</th>
		<th align=left>dl count</th>
		<th align=left>max dl count</th>
		<th align=left>delete</th>
	</tr>
	<?php foreach($data as $i=>$row):?>
		<tr bgcolor="<?=$color[$ck = ~$ck];?>">
			<td style="padding-left:2px;">
			<input type="hidden" name="dlid[]" value="<?=$row['dlid'];?>">
				<input type="text" size="10" name="order_number[]" value="<?=$row['order_number'];?>">
			</td>
			<td style="padding-left:2px;"><input type="text" size="20" name="email[]" value="<?=$row['email'];?>"></td>
			<td style="padding-left:2px;"><input type="text" size="15" name="filename[]" value="<?=$row['filename'];?>"></td>
			<td style="padding-left:2px;"><input type="text" size="35" name="url[]" value="<?=$row['url'];?>"></td>
			<td style="padding-left:2px;">
					<?=$_Common->makeSimpleSelectBox("complete[]",array('true','false'),array('true','false'),$row['complete']);?>
			</td>
			<td style="padding-left:2px;"><input type="text" size="10" name="expire_date[]" value="<?=$row['expire_date'];?>"></td>
			<td style="padding-left:2px;"><input type="text" size="10" name="dl_count[]" value="<?=$row['dl_count'];?>"></td>
			<td style="padding-left:2px;"><input type="text" size="10" name="max_download_count[]" value="<?=$row['max_download_count'];?>"></td>
			<td align=center valign="middle" width="50"><a href="downloads.php?delete=true&dlid=<?=$row['dlid'];?>" onclick="return confirm('Are you sure you want to delete this download?')"><img src="../icons/trash.gif" border="0" alt="Delete"></a></td>
		</tr>
	<?php endforeach;?>
	</table>
	
	<p align="center"><input type="submit" name="update" value="Update Downloads"></p>
</form>

</body>
</html>