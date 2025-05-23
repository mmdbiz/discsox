<?php
//VersionInfo:Version[3.0.1]

// TO DO: Check to make sure username/password don't already exist
//        before adding or updating records.

$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$debug = false;
$customerTypes = array();
$error = NULL;
$fldProperties = $_DB->getFieldProperties("customers");
$shipFlds = $_DB->getFieldProperties("customer_shipping");
$maxToDisplay = 25;
$hits = 0;
$start = 0;
$end = 0;
$count = 0;
$links = "";
$records = array();
$orderCounts = array();

//$_Common->debugPrint($_REQUEST);
//exit;


if(!empty($_REQUEST['insert']) || !empty($_REQUEST['update']) || !empty($_REQUEST['delete'])){
	update();
}

// For Pull downs
global $provinces;
global $states;
global $countries;
include_once("../../include/countries.inc");
include_once("../../include/provinces.inc");
include_once("../../include/states.inc");

// -------------------------------------------------------------------
function add(){
	global $_Common;
	global $_DB;
	global $fldProperties;
	global $records;
	foreach($fldProperties as $key=>$props){
		$records[$key] = $_DB->getDefaultValues($key);
	}
	$_Common->loadStateCountry($records);	

}

// -------------------------------------------------------------------
// Checks the customer table for matching username/password records
// before adding or updating the records. No duplicates allowed.
// Also sets passwords to MD5 if encrypt_password is on in config.
function checkCustomer(&$formFields,$cid = ""){
	
	global $_CF;
	global $_DB;
	global $_Common;
    
    $user = null;
    $pass = null;
    
    if(!empty($_REQUEST['username'])){
	    $user = trim($_REQUEST['username']);
	}
	if(!empty($_REQUEST['username'])){
		$pass = trim($_REQUEST['password']);
	}
	
	// No username to test against. Must not be filled in
	if(!$user){
		return true;	
	}
    $sql = "SELECT cid FROM customers WHERE username = '$user'";
    
	if($pass){
		if(strlen($pass) < 32 && $_CF['login']['encrypt_password'] == 'true'){
			$_REQUEST['password'] = md5($pass);
			$pass = md5($pass);
		}
		$sql .= " AND password = '$pass'";
	}
	
	if(count($formFields) > 0){
		$formFields['username'] = $user;
		$formFields['password'] = $pass;
	}
		
	$rs = $_DB->execute($sql);
	$rCount = $_DB->numrows($rs);
	
	if($rCount > 1){
		// error. 2 people have the same username/password.
		$_Common->printErrorMessage("Username/Password Error",
		"More than one customer is using the same username/password. This needs to be corrected.");
	}
	elseif($rCount == 1){
		// this is OK for updates
		$row = $_DB->fetchrow($rs, "ASSOC");
		if($cid != "" && $row['cid'] == $cid){
			return true;	
		}
		else{
			$_Common->printErrorMessage("Username/Password Error",
			"This username/password is already being used by a different customer.");
		}
	}
	else{
		return true;
	}
}
	
// -------------------------------------------------------------------
function update(){

	global $_Common;
	global $_DB;
	global $debug;
	global $fldProperties;
	global $shipFlds;
		
	$custFields = $fldProperties;

	$delete = false;
	$insert = false;
	$edit = false;
	
	if(!empty($_REQUEST['delete'])){
		$delete = true;	
	}
	$multi = false;

	if(!empty($_REQUEST['insert'])){
		$insert = true;
		$blankData = array();
		if(checkCustomer($blankData)){

			list($fields, $values) = $_DB->makeAddFields($custFields, 'cid', $_REQUEST);
	        
			if($debug){
				$_Common->debugPrint($sql,"Insert SQL");
			}            
	        
			$sql = "INSERT INTO customers ($fields) VALUES ($values)";
			$custInsert = $_DB->execute($sql);
			$cid = $_DB->lastInsertID();
			if($debug){
				print "<pre>NewCustID: $cid\n";
			}
			$_POST['cid'] = $cid;
		}
	}
	else{

        if(!empty($_POST['cid']) && is_array($_POST['cid'])){

			//$_Common->debugPrint($_POST,"do multi update");

            $multi = true;
            foreach($_POST['cid'] as $cid=>$sel){

                $formFields = array();
                $formFields['cid'] = $cid;

                foreach($_REQUEST as $key=>$arrayVal){
                    if(is_array($arrayVal) && isset($_REQUEST[$key][$cid])){
                        $formFields[$key] = $_REQUEST[$key][$cid];
                    }
                }

                if($delete){
					deleteCustomerRecords($cid);
                }
                else{
					$edit = true;
					if(checkCustomer($formFields,$cid)){
						$values = $_DB->makeUpdateFields($custFields, 'cid', $formFields);
						$sql = "UPDATE customers SET $values WHERE cid = '$cid'";
						if($debug){
							$_Common->debugPrint($sql,"Update SQL");
						}   
						$custUpdate = $_DB->execute($sql);
					}
                }
            }
        }
        else{
			//$_Common->debugPrint($_POST,"do single update");

			if(!empty($_GET['cid'])){
				$_POST['cid'] = $_GET['cid'];
			}

			if(!empty($_POST['cid']) && !is_array($_POST['cid'])){
				
				// cid is NOT an array. Single record
				$cid = $_POST['cid'];
				
				if($delete){
					deleteCustomerRecords($cid);
				}
				else{
					$edit = true;
					if(checkCustomer($_REQUEST,$cid)){
						$values = $_DB->makeUpdateFields($custFields, 'cid', $_REQUEST);
						$sql = "UPDATE customers SET $values WHERE cid = '$cid'";
						if($debug){
							$_Common->debugPrint($sql,"Update SQL");
						}   
						$custUpdate = $_DB->execute($sql);
						
						$_REQUEST['cid'] = $cid;
						// Update customer shipping
						if(!empty($_REQUEST['csid'])){
							$csid = $_REQUEST['csid'];
							// delete the selected record
							if(!empty($_REQUEST['delete_shipping'])){
								$sql = "DELETE FROM customer_shipping WHERE cid = '$cid' AND csid = '$csid' LIMIT 1";
								if($debug){
									$_Common->debugPrint($sql,"Delete Customer Shipping SQL");
								}   
								$_DB->execute($sql);
								unset($_REQUEST['csid']);
							}
							else{
								// update the record
								$values = $_DB->makeUpdateFields($shipFlds, 'csid', $_REQUEST);
								$sql = "UPDATE customer_shipping SET $values WHERE cid = '$cid' AND csid = '$csid'";
								if($debug){
									$_Common->debugPrint($sql,"Update Customer Shipping SQL");
								}   
								$shipUpdate = $_DB->execute($sql);
							}
						}
						elseif(!empty($_REQUEST['add_shipping'])){
							// add as a new record
							list($fields, $values) = $_DB->makeAddFields($shipFlds, 'csid', $_REQUEST);
							$sql = "INSERT INTO customer_shipping ($fields) VALUES ($values)";
							if($debug){
								$_Common->debugPrint($sql,"customer_shipping insert SQL");
							}
							$_DB->execute($sql);
							$csid = $_DB->lastInsertID();
							$_REQUEST['csid'] = $csid;
						}
					}
				}
			}
        }	
	}

	$_REQUEST['update'] = NULL;
	unset($_REQUEST['update']);
	
	if($delete){
        $_REQUEST['delete'] = NULL;
        unset($_REQUEST['delete']);
    }
	if($multi || $delete){
		$_REQUEST['list'] = true;
	}
	elseif($insert){
		$_REQUEST['insert'] = NULL;
		unset($_REQUEST['insert']);
		$_REQUEST['modify'] = true;
	}
	else{
		$_REQUEST['modify'] = true;
	}
	
}

// -------------------------------------------------------------------
function deleteCustomerRecords($cid){
	
	global $_DB;
	
	$sql = "DELETE FROM customers WHERE cid = '$cid'";
	$custDelete = $_DB->execute($sql);
	$sql = "DELETE FROM customer_shipping WHERE cid = '$cid'";
	$custDelete = $_DB->execute($sql);
	$sql = "DELETE FROM customer_favorites WHERE cid = '$cid'";
	$custDelete = $_DB->execute($sql);
	
	// Get a list of order id's to recursively delete
	$sql = "SELECT orid FROM orders WHERE cid = '$cid'";
	$oridRS = $_DB->execute($sql);
	$orids = array();
	while($row = $_DB->fetchrow($oridRS, "ASSOC")){
		$orids[] = $row['orid'];
	}
	$oridList = "'" . join("','",$orids) . "'";
	
	// now we can delete orders
	$sql = "DELETE FROM orders WHERE cid = '$cid'";
	$custDelete = $_DB->execute($sql);
	
	// get all the detail ids so we can delete order options
	$sql = "SELECT ordid FROM order_details WHERE orid IN($oridList)";
	$detailRS = $_DB->execute($sql);
	$ordids = array();
	while($row = $_DB->fetchrow($detailRS, "ASSOC")){
		$ordids[] = $row['ordid'];
	}
	$detailIdList = "'" . join("','",$ordids) . "'";
	
	$sql = "DELETE FROM order_details WHERE orid IN($oridList)";
	$custDelete = $_DB->execute($sql);
	
	$sql = "DELETE FROM order_options WHERE ordid IN($detailIdList)";
	$custDelete = $_DB->execute($sql);
	
	
}





// -------------------------------------------------------------------
function getCustomer($fields = "*",$custNum = null){

    global $_Common;
    global $count;
	global $_DB;
	global $debug;
	global $end;
	global $error;
	global $hits;
	global $limit;
	global $links;
	global $maxToDisplay;
	global $orderCounts;
	global $records;
	global $start;
	global $searchFields;
	


	
	// get a count of customers
    $count = $_DB->getCount('customers');

	if($count > 0){

		$RS = $_DB->execute("SHOW FIELDS FROM customers LIKE 'billaddress%'");
		while($row = $_DB->fetchrow($RS)){
			$fldName = strtolower($row[0]);
			$searchFields[$fldName] = 1;
		}
		$_DB->free_result($RS);
		$searchFields['customer_number'] = 1;

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

		$sql = "SELECT $fields FROM customers";
	    $where = "";
	    
	    if(!empty($_GET['cid'])){
			$cid = $_GET['cid'];
			$where = "WHERE cid = '$cid' LIMIT 1";
		}
		elseif($custNum){
			$where = "WHERE customer_number = '$custNum' LIMIT 1";
		}
		else{
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
			if(!empty($_REQUEST['sort'])){
				$sort = trim($_REQUEST['sort']);
				$sortDirection = "ASC";
				if(isset($_SESSION['sort_by']) && $_SESSION['sort_by'] == $sort){
					
					if(isset($_SESSION['sort_dir']) && $_SESSION['sort_dir'] == "DESC"){
						$where .= " ORDER BY $sort ASC";
						$sortDirection = "DESC";
					}
					else{
						$where .= " ORDER BY $sort DESC";
						$sortDirection = "ASC";
					}
				}
				else{
					$where .= " ORDER BY $sort ASC";
				}
				$_SESSION['sort_by'] = $sort;
				$_SESSION['sort_dir'] = $sortDirection;
			}
			elseif(isset($_SESSION['sort_by'])){
				$sort = $_SESSION['sort_by'];
				if(isset($_SESSION['sort_dir']) && $_SESSION['sort_dir'] == "DESC"){
					$where .= " ORDER BY $sort ASC";
					$sortDirection = "ASC";
				}
				else{
					$where .= " ORDER BY $sort DESC";
					$sortDirection = "DESC";
				}
				$_SESSION['sort_dir'] = $sortDirection;
			}
			else{
				$where .= " ORDER BY billaddress_companyname ASC";
			}
		}

		if(stristr($where,'ORDER BY customer_name')){
			$thisWhere = str_replace('customer_name','billaddress_firstname',$where);
			$count = $_DB->getCount('customers',$thisWhere);
		}
		else{
			$count = $_DB->getCount('customers',$where);
		}

		list($start,$end,$limit) = $_DB->getLimits($count,$max,"customers.php");
		$links = $_DB->previousNextLinks;

		$sql .= " $where";

		if($limit && !$custNum){
			$sql .= $limit;
		}

		$records = $_DB->getRecords($sql);

//$_Common->debugPrint($records,$sql);

		if(count($records) > 0){
			$ids = array();
			foreach($records as $i=>$flds){
				$nums[] = $flds['customer_number'];
				$ids[] = $flds['cid'];
				
				if($custNum){
					$_Common->loadStateCountry($records[$i]);
				}				
			}
			if(empty($_REQUEST['detail'])){
				$_SESSION['nums_list'] = $nums;
			}
			$idList = "'" . join("','",$ids) . "'";
			$sql = "SELECT cid, COUNT(cid) as totalOrders, SUM(grandtotal) as total FROM orders WHERE cid IN($idList) GROUP BY cid";
			$rs = $_DB->execute($sql);
			while($row = $_DB->fetchrow($rs,"ASSOC")){
				$ocid = $row['cid'];
				$orderCounts[$ocid]['count'] = $row['totalorders'];
				$orderCounts[$ocid]['total'] = $row['total'];
			}
		}

		if($debug){
			$_Common->debugPrint($sql);
			$_Common->debugPrint($records);
		}

		if(count($records) == 0){
			$error = "Customer information not found.";
		}
		else{
			if(isset($_REQUEST['modify'])){
				$records = $records[0];	
			}	
		}
	}
	else{
		$error = "Customer information not found.";
	}
}

// -------------------------------------------------------------------
function getCustomerShipping($cid){
	
    global $_Common;
	global $_DB;
	global $debug;
	global $shipData;
	global $shipFlds;
	global $shipSelect;
	global $shipCount;

	$sql = "SELECT * FROM customer_shipping WHERE cid = '$cid'";	
	$data = $_DB->getRecords($sql);	

	$shipCount = 0;
	$shipSelect = null;
	$csid = null;
	
	if(!empty($_REQUEST['csid'])){
		$csid = $_REQUEST['csid'];
	}
	if(count($data) > 0){
		$shipCount = count($data);
		$csids = array();
		$names = array();
		foreach($data as $i=>$row){
			if($csid && $row['csid'] == $csid){
				$shipData = $row;
			}
			$csids[] = $row['csid'];
			$names[] = $row['shipaddress_addr1'] . ', ' . $row['shipaddress_city'];
		}
		if(!$csid){
			$shipData = $data[0];
		}
		list($shipSelect,$selected) = $_Common->makeSelectBox('csid',$names,$csids,$csid,true);

		$_Common->loadStateCountry($shipData);
	}
	else{
		foreach($shipFlds as $key=>$props){
			$shipData[$key] = $_DB->getDefaultValues($key);
		}
		$_Common->loadStateCountry($shipData);		
	}

	if($debug){
		$_Common->debugPrint($shipData,"Selected Customer Shipping");
	}
}





// -------------------------------------------------------------------
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

// -------------------------------------------------------------------
function detailPreviousNext($cid){
	
	global $_Common;
	
	$previous = null;
	$next = null;

//$_Common->debugPrint($_SESSION['nums_list'],"nums");
	
	if(!empty($_SESSION['nums_list'])){
		$idList = $_SESSION['nums_list'];
		foreach($idList as $i=>$id){
			if($id == $cid){
				if(isset($idList[$i-1])){
					$previous = $idList[$i-1];
				}
				if(isset($idList[$i+1])){
					$next = $idList[$i+1];
				}
				break;
			}
		}
	}
	
	//$_Common->debugPrint(array($previous,$next),"nums");
	
	return array($previous,$next);	
}

error_reporting(E_ALL ^ E_NOTICE);
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

function clearShipping(form){
	for(i = 0; i < form.elements.length; i++){
		var fldType = form.elements[i].type;
		var fldName = form.elements[i].name;
		if(fldType == "text" && fldName.substring(0,11) == "shipaddress"){
			form.elements[i].value = "";
		}
	}
	if(form.elements['csid'].type == "select-one"){
		var sIndex = form.elements['csid'].selectedIndex;
		form.elements['csid'].options[sIndex].value = "";
	}
	if(form.elements['csid'].type == "hidden"){
		form.elements['csid'].value = "";
	}
}
//-->
</script>
<script LANGUAGE="JavaScript" src="../javascripts/reports.js"></script>
<script LANGUAGE="JavaScript" src="../javascripts/popcalendar.js"></script>
</head>
<body class="mainBody">
	<div align="center">
		<form name="customer" method="post" action="customers.php">


		<?php if(!empty($_REQUEST['add'])):?>

			<?php add();?>

			<table border="0" cellpadding="3" cellspacing="0" width="600">
				<tr>
					<th colspan="2" align="left">Add New Customer</th>
				</tr>
				<tr>
					<td colspan="2" align="left">&nbsp;</td>
				</tr>
				
				<?php foreach($records as $key=>$value):?>
					<?php 
						if($key == "cid"){
							continue;	
						}
						$label = ucwords(str_replace("_"," ",$key));
					?>
					<tr>
						<td align="right" valign="top" width="30%"><?=$label;?>: </td>
						<td align="left">
						
							<?php if(stristr($value,"<select")):?>
								<?=$value;?>

							<?php elseif(stristr($key,"date")):?>
								<input type=text name="<?=$key;?>" value="<?=$value;?>" size=10>
								<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="popUpCalendar(this, document.getElementById('<?=$key;?>'), 'yyyy-mm-dd', 0, 0)">

							<?php elseif(stristr($key,"comments") || stristr($key,"notes")):?>
								<textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea>

							<?php else:?>
								<input type=text name="<?=$key;?>" value="<?=$value;?>" size=50>

							<?php endif;?>
						</td>
					</tr>
				<?php endforeach;?>
			</table>

			<p><input type="submit" name="insert" value="Add Customer"></p>


		<?php elseif(!empty($_REQUEST['modify']) && !empty($_REQUEST['customer_number'])):?>

			<?php getCustomer('*',$_REQUEST['customer_number']); ?>

			<?php if($error):?>
				<p><br /><b><?=$error;?></b></p>
			<?php else:?>

				<?php if(isset($records['cid'])):?>
					<input type="hidden" name="cid" value="<?=$records['cid'];?>">
					<input type="hidden" name="modify" value="true">
					<?php 
						getCustomerShipping($records['cid']);
						@list($previous,$next) = detailPreviousNext($records['customer_number']);
						$customer_number = $_REQUEST['customer_number'];
					?>
				<?php endif;?>

				<table border="0" cellpadding="4" cellspacing="0" width="600">
					<tr>
						<th colspan="2" align="left">Edit Customer</th>
					</tr>

					<?php if(!is_null($previous) || !is_null($next)):?>
						<tr>
							<td colspan="2" align="left">
								<table border="0" cellspacing="0" cellpadding="0" width="100%">
									<tr>
										<td align="left" valign="top">
											<?php if(!is_null($previous)):?>
												<a href="customers.php?modify=true&customer_number=<?=$previous;?>&detail=true"><img border="0" src="images/nav/arrow_previous_on.gif"> Previous Customer</a>
											<?php else:?>
												&nbsp;
											<?php endif;?>
										</td>
										<td align="right" valign="top">
											<?php if(!is_null($next)):?>
												<a href="customers.php?modify=true&customer_number=<?=$next;?>&detail=true">Next Customer <img border="0" src="images/nav/arrow_next_on.gif"></a>
											<?php else:?>
												&nbsp;
											<?php endif;?>
										</td>
									</tr>
								</table>
							</td>
						</tr>
					<?php endif;?>
				
					<tr>
						<td colspan="2" align="left">&nbsp;</td>
					</tr>

					<?php if(isset($orderCounts[$records['cid']])):?>
					<tr>
						<td align="right" valign="top" width="30%">Order Count: </td>
						<td align="left">
							<a href="orders.php?cid=<?=$records['cid'];?>&amp;customer_number=<?=$records['customer_number'];?>"><u><?=$orderCounts[$records['cid']]['count'];?> orders</u></a>
						</td>
					</tr>
					<tr>
						<td align="right" valign="top" width="30%">Order Totals: </td>
						<td align="left">
							<?=number_format($orderCounts[$records['cid']]['total'],2);?>
						</td>
					</tr>
					<?php endif;?>

					<?php foreach($records as $key=>$value):?>
						<?php 
							if($key == "cid"){
								continue;	
							}
							else{
								$value = $_DB->getDefaultValues($key,$value);	
							}
							$label = ucwords(str_replace("_"," ",$key));
						?>
						<tr>
							<?php if(stristr($key,"notes")):?>
								<td align="right" valign="top" width="30%"><?=$label;?>: </td>
							<?php else:?>
								<td align="right" valign="middle" width="30%"><?=$label;?>: </td>
							<?php endif;?>
							<td align="left">
								<?php if(stristr($value,"<select")):?>
									<?=$value;?>
								<?php elseif(stristr($key,"date")):?>
									<input type=text name="<?=$key;?>" value="<?=$value;?>" size=10>
									<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="popUpCalendar(this, document.getElementById('<?=$key;?>'), 'yyyy-mm-dd', 0, 0)">
								<?php elseif(stristr($key,"comments") || stristr($key,"notes")):?>
									<textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea>
								<?php else:?>
									<input type=text name="<?=$key;?>" value="<?=$value;?>" size=50>
								<?php endif;?>
							</td>
						</tr>
					<?php endforeach;?>
				</table>
				<br />

				<?php if($shipCount > 0):?>

				
					<?php $fldProperties = $shipFlds;?>
					
					<table border="0" cellpadding="4" cellspacing="0" width="600">
						<tr>
							<th colspan="2" align="left">Customer Shipping</th>
						</tr>
						<tr>
							<td colspan="2" align="left">&nbsp;</td>
						</tr>
						
						<?php if($shipCount > 1):?>
						<tr>
							<td align="right" valign="middle" width="30%">Address Book: </td>
							<td align="left">
								<?=$shipSelect;?>
							</td>
						</tr>
						<?php elseif($shipCount == 1):?>
							<input type="hidden" name="csid" value="<?=$shipData['csid'];?>">
						<?php endif;?>
						
						<tr>
							<td align="right" valign="middle" width="30%">Delete this shipping address: </td>
							<td align="left">
								<input type="checkbox" name="delete_shipping" value="true"> 
							</td>
						</tr>
						<tr>
							<td align="right" valign="middle" width="30%">Add new shipping address: </td>
							<td align="left">
								<input type="checkbox" name="add_shipping" value="true" onClick="clearShipping(this.form);"> 
							</td>
						</tr>
						<?php foreach($shipData as $key=>$value):?>
							<?php 
								if($key == "csid" || $key == "cid"){
									continue;	
								}
								else{
									$value = $_DB->getDefaultValues($key,$value);	
								}
								$label = ucwords(str_replace("_"," ",$key));
							?>
							<tr>
								<td align="right" valign="middle" width="30%"><?=$label;?>: </td>
								<td align="left">
									<?php if(stristr($value,"<select")):?>
										<?=$value;?>
									<?php elseif(stristr($key,"date")):?>
										<input type=text name="<?=$key;?>" value="<?=$value;?>" size=10>
										<img src="../images/calendaricon.gif" height="17" width="17" border=0 onClick="popUpCalendar(this, document.getElementById('<?=$key;?>'), 'yyyy-mm-dd', 0, 0)">
									<?php elseif(stristr($key,"comments") || stristr($key,"notes")):?>
										<textarea name="<?=$key;?>" rows="5" cols="49" wrap="virtual"><?=$value;?></textarea>
									<?php else:?>
										<input type=text name="<?=$key;?>" value="<?=$value;?>" size=50>
									<?php endif;?>
								</td>
							</tr>
						<?php endforeach;?>
					</table>

				<?php endif;?>


				<p>
					<?php if(!is_null($previous)):?>
						<input type="button" name="previous" value="Previous Customer" onClick="location.href='customers.php?modify=true&customer_number=<?=$previous;?>&detail=true'";> &nbsp;
					<?php endif;?>
					
					<input type="submit" name="update" value="Update Customer">
					
					<?php if(!is_null($next)):?>
						&nbsp; <input type="button" name="next" value="Next Customer" onClick="location.href='customers.php?modify=true&customer_number=<?=$next;?>&detail=true'";>
					<?php endif;?>
					
				</p>

			<?php endif;?>

		<?php else:?>

			<?php
				$flds = "cid,billaddress_companyname,CONCAT(billaddress_firstname,' ',billaddress_lastname) AS customer_name,customer_number,active,customer_type";
				getCustomer($flds);
				$color = array();
				$color[0] = "#E2EDE2";
				$color[~0] = "#FFFFFF";
				$ck = 0;
			?>
			<?php if($error):?>
				<p><br /><b><?=$error;?></b></p>
				<p><br /><input type="submit" name="add" value="Add New Customer"></p>
			<?php else:?>
			
				<h4><br />Customer List</h4>

				<table border="0" cellpadding="3" cellspacing="0" width="700">

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
					<td align="left">
						<a href="javascript:selectAll(document.forms['customer'],true);">Select All</a>&nbsp;-&nbsp;
						<a href="javascript:selectAll(document.forms['customer'],false);">Unselect All</a>
					</td>
					<td align="right"><?=$links;?></td>
				</tr>
				<tr><td colspan="2">&nbsp;</td></tr>
				</table>
				
				<table border="0" cellpadding="3" cellspacing="1" width="700">
				<tr>
					<th nowrap>Select</th>
					<th nowrap>
						<a href="customers.php?hits=<?=$hits;?>&amp;max=<?=$maxToDisplay;?>&sort=active"><font color="white">Active</font></a>
					</th>
					<th nowrap>
						<a href="customers.php?hits=<?=$hits;?>&amp;max=<?=$maxToDisplay;?>&sort=customer_number"><font color="white">Number</font></a>
					</th>
					<th nowrap>
						<a href="customers.php?hits=<?=$hits;?>&amp;max=<?=$maxToDisplay;?>&sort=customer_name"><font color="white">Customer Name</a>
					</th>
					<!--th nowrap>
						<a href="customers.php?hits=<?=$hits;?>&amp;max=<?=$maxToDisplay;?>&sort=customer_type"><font color="white">Type</a>
					</th-->
					<th nowrap>Orders</th>
					<th nowrap>Totals</th>
				</tr>

				<?php foreach($records as $i=>$data):?>

					<?php
					
						//$_Common->debugPrint($data);
					
						list($select,$selected) = $_Common->makeSelectBox("active[" . $data['cid'] . "]",array('true','false'),array('true','false'),$data['active']);
						$data['active'] = $select;
						if(trim($data['billaddress_companyname']) != ""){
							$data['customer_name'] = $data['billaddress_companyname'];
						}
					?>

					<tr bgcolor="<?=$color[$ck = ~$ck];?>">
						<td align="center" width="50">
							<input type="checkbox" name="cid[<?=$data['cid'];?>][selected]" value="true">
						</td>
						<td align="center">
							<?=$data['active'];?>
						</td>
						<td align="center" width="50">
							<a href="customers.php?modify=true&amp;cid=<?=$data['cid'];?>&amp;customer_number=<?=$data['customer_number'];?>&amp;detail=true"><u><?=$data['customer_number'];?></u></a>
						</td>
						<td align="left">
							<?=$data['customer_name'];?>
						</td>
						<!--td align="left">
							<?=$data['customer_type'];?>
						</td-->
						
						<?php if(isset($orderCounts[$data['cid']])):?>
							<td align="center">
								<a href="orders.php?cid=<?=$data['cid'];?>&amp;customer_number=<?=$data['customer_number'];?>"><u><?=$orderCounts[$data['cid']]['count'];?></u></a>
							</td>
							<td align="right">
								<?=number_format($orderCounts[$data['cid']]['total'],2);?>
							</td>
						<?php else:?>
							<td align="center">0</td>
							<td align="right"><?=number_format(0,2);?></td>
						<?php endif;?>
						
					</tr>
				
				<?php endforeach;?>
			
				</table>

				<p><br />
					<input type="submit" name="update" value="Update Selected Records" onClick="return checkSelected(document.forms['customer'],'Update','customer');">
					<input type="submit" name="add" value="Add New Customer">
					<input type="submit" name="delete" value="Delete Selected Records" onClick="return checkSelected(document.forms['customer'],'delete','customer');">
				</p>
			
			<?php endif;?>

		<?php endif;?>

		</form>
		<p>&nbsp;</p>
	</div>
</body>
</html>


