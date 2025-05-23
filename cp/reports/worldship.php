<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "reports";

// initialize the program and read the config(s)
include_once("../../include/initialize.inc");
$init = new Initialize(true);

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$orderFlds = $_DB->getFieldProperties('orders');
if(isset($orderFlds['tracking_number'][1]) && $orderFlds['tracking_number'][1] != 'text'){
	$_DB->execute("ALTER TABLE `orders` CHANGE `tracking_number` `tracking_number` TEXT");
}


$error = null;
$message = null;

$services = array("UPS Ground",
				  "UPS 2nd Day Air",
				  "UPS Next Day Air",
				  "UPS Worldwide Express",
				  "UPS Worldwide Expedited",
				  "UPS Standard",
				  "UPS 3 Day Select",
				  "UPS Next Day Air Saver",
				  "UPS Next Day Air Early A.M.",
				  "UPS Worldwide Express Plus",
				  "UPS 2nd Day Air A.M.",
				  "UPS Express Saver");

if(isset($_POST['download'])){
	
	$idList = null;
	if(isset($_POST['checked'])){
		//debugPrint($_POST['checked']);
		$idList = "'" . join("','",array_keys($_POST['checked'])) . "'";
	}

	$sql = "SELECT customer_shipping.*,
				   CONCAT(customer_shipping.shipaddress_firstname, ' ', customer_shipping.shipaddress_lastname) AS shipaddress_name,
				   CONCAT(customer_shipping.shipaddress_areacode, '-', customer_shipping.shipaddress_phone) AS shipaddress_phone,
				   orders.order_number,
				   orders.grandtotal,
				   orders.shipping_method,
				   SUM(order_details.weight * order_details.quantity) AS weight
			FROM customer_shipping,orders,order_details
			WHERE customer_shipping.csid = orders.csid 
			AND order_details.orid = orders.orid";
			if($idList){
				$sql .= " AND orders.order_number IN($idList)";
			}
			else{
				$sql .= " AND orders.status != 'complete'";
			}
			$sql .= " GROUP BY orders.orid ORDER BY orders.order_number";
			
	$records = $_DB->getRecords($sql);

	// build csv and download.
	if(count($records > 0)){
		
		$csvFields = array('ORDERID'											=> 'order_number',
						'PACKAGE_DECLAREDVALUEAMOUNT'							=> 'grandtotal',
						'PACKAGE_DECLAREDVALUEOPTION'							=> '',
						'PACKAGE_PACKAGETYPE'									=> '',
						'PACKAGE_REFERENCE1'									=> 'order_number',
						'PACKAGE_REFERENCE2'									=> '',
						'PACKAGE_REFERENCE3'									=> '',
						'PACKAGE_REFERENCE4'									=> '',
						'PACKAGE_REFERENCE5'									=> '',
						'PACKAGE_WEIGHT'										=> 'weight',
						'SHIPMENTINFORMATION_BILLINGOPTION'						=> '',
						'SHIPMENTINFORMATION_NOTIFICATIONRECIPIENT1FAXOREMAIL'	=> 'shipaddress_email',
						'SHIPMENTINFORMATION_NOTIFICATIONRECIPIENT1TYPE'		=> 'EMAIL',
						'SHIPMENTINFORMATION_QVNOPTION'							=> 'Y',
						'SHIPMENTINFORMATION_QVNSHIPNOTIFICATION1OPTION'		=> 'Y',
						'SHIPMENTINFORMATION_SERVICETYPE'						=> 'shipping_method',
						'SHIPTO_CITY'											=> 'shipaddress_city',
						'SHIPTO_COMPANYORNAME'									=> 'shipaddress_name',
						'SHIPTO_COUNTRY'										=> 'shipaddress_country',
						'SHIPTO_LOCATIONID'										=> '',
						'SHIPTO_RESIDENTIALINDICATOR'							=> 'shipaddress_delivery_type',
						'SHIPTO_ROOMFLOORADDRESS2'								=> 'shipaddress_addr2',
						'SHIPTO_STATE'											=> 'shipaddress_state',
						'SHIPTO_STREETADDRESS'									=> 'shipaddress_addr1',
						'SHIPTO_TELEPHONE'										=> 'shipaddress_phone',
						'SHIPTO_ZIPCODE'										=> 'shipaddress_postalcode');
						
						
		$headers = array_keys($csvFields);
		$rows = array();
		foreach($records as $i=>$data){
			
			if($data['weight'] == 0){
				$data['weight'] = 1;
			}
			
			if((stristr($data['shipping_method'],"UPS") || stristr($data['shipping_method'],"Ground")) && !in_array($data['shipping_method'],$services)){
				$data['shipping_method'] = "UPS Ground";
			}			
			elseif(!in_array($data['shipping_method'],$services)){
				continue;
			}

			if(isset($data['shipaddress_delivery_type']) && trim($data['shipaddress_delivery_type']) == "residential"){
				$data['shipaddress_delivery_type'] = "yes";
			}
			else{
				$data['shipaddress_delivery_type'] = "no";
			}

			if(isset($data['shipaddress_companyname']) && trim($data['shipaddress_companyname']) != ""){
				$data['shipaddress_name'] = $data['shipaddress_companyname'];
			}
			
			$flds = array();
			foreach($csvFields as $ws=>$fld){
				if(isset($data[$fld])){
					$flds[$ws] = $data[$fld];
				}
				else{
					$flds[$ws] = $fld;
				}
			}
			$rows[$i] = $flds;	
		}
		downloadReport($headers,$rows);
		exit;
	}
}
elseif(isset($_POST['update'])){
	
	// update records as shipped
	if(isset($_POST['checked'])){
		//debugPrint($_POST['checked']);
		$idList = "'" . join("','",array_keys($_POST['checked'])) . "'";
		//debugPrint($idList);
		$sql = "UPDATE orders SET status = 'Complete' WHERE order_number IN($idList)";
		//debugPrint($sql);
		$_DB->execute($sql);
	}
	
}
elseif(!empty($_FILES['tracking_file']) && $_FILES['tracking_file']['error'] == 0 && $_FILES['tracking_file']['size'] > 0){

	$fName = trim($_FILES['tracking_file']['name']);
	$info = pathinfo($fName);
	if($info['extension'] == "csv"){
		
		$data = file($_FILES['tracking_file']['tmp_name']);
		unlink($_FILES['tracking_file']['tmp_name']);
		//$_Common->debugPrint($data);
		//exit;
		
		$date = date("Y-m-d");
		foreach($data as $row){
			$flds = explode(',',$row);
			if(count($flds) > 1){
				$tNumber = trim($flds[0]);
				if(strstr($flds[1],'.')){
					$orderFld = explode('.',trim($flds[1]));
				}
				elseif(strstr($flds[1],'_')){
					$orderFld = explode('.',trim($flds[1]));
				}
				$ordNum = array_pop($orderFld);
				
				$order = array();
				if(trim($ordNum) != "" && $tNumber != ""){
					$order = $_DB->getRecord("SELECT number_of_packages,tracking_number,orid FROM orders WHERE `order_number` = '$ordNum'");
				}
				if(count($order) == 0){
					$error .= "Order number $ordNum was not found in the database<br />";
				}
				else{
					$tNumbers = array();
					if(!empty($order['tracking_number'])){
						$tNumbers = explode(',',$order['tracking_number']);
					}
					if(!in_array($tNumber,$tNumbers)){
						$tNumbers[] = $tNumber;
						
						$status = "Partial Shipped";
						if(!empty($order['number_of_packages']) && count($tNumbers) == $order['number_of_packages']){
							$status = "Complete";
						}
						
						$tNumber = join(',',$tNumbers);
						
						$sql = "UPDATE orders SET `tracking_number`='$tNumber',`status`='$status',`date_shipped`='$date' WHERE `order_number`='$ordNum'";
						$_Common->debugPrint($sql);
						if($_DB->execute($sql)){
							$message = "The orders have been updated";
						}
						else{
							$error .= "Order number $ordNum has not been updated<br />";
						}
					}
					else{
						$error .= "The tracking number: $tNumber for Order number $ordNum already exists<br />";						
					}
				}
			}
		}
	}
	else{
		$error = "Only .csv files are allowed to be uploaded for Worldship";
	}
}

//$_Common->debugPrint($services,$_CF['shipping']['free_shipping_text']);

// show list
$sql = "SELECT orders.status,
			   orders.order_number,
			   orders.transaction_date,
			   CONCAT(customer_shipping.shipaddress_firstname, ' ', customer_shipping.shipaddress_lastname) AS shipaddress_name,
			   customer_shipping.shipaddress_addr1,
			   customer_shipping.shipaddress_addr2,
			   customer_shipping.shipaddress_city,
			   customer_shipping.shipaddress_state,
			   customer_shipping.shipaddress_country,
			   customer_shipping.shipaddress_postalcode,
			   CONCAT(customer_shipping.shipaddress_areacode, '-', customer_shipping.shipaddress_phone) as shipaddress_phone,
			   customer_shipping.shipaddress_email,
			   orders.shipping_method
		FROM orders, customer_shipping
		WHERE customer_shipping.csid = orders.csid
		AND orders.status != 'complete'
		ORDER BY orders.order_number";

$records = $_DB->getRecords($sql);

if(count($records > 0)){
	$data = array();
	foreach($records as $j=>$row){
		if(in_array($row['shipping_method'],$services)){
			$data[] = $row;
		}
		elseif(stristr($row['shipping_method'],"UPS") || stristr($row['shipping_method'],"Ground")){
			$data[] = $row;
		}
	}
	$records = $data;
}


//-----------------------------------------------------
function downloadReport($headers,$records){

    $header = trim(join(',',$headers));

    $data = "";
    foreach($records as $index=>$row){
		$data .= '"' . join('","',array_values($row)) . '"' . "\n";
    }
    $data = str_replace("\r", "", $data);

	$date = date("Y-m-d");

    # This line will stream the file to the user rather than spray it across the screen
    header("Content-type: application/octet-stream");

    # replace excelfile.xls with whatever you want the filename to default to
    header("Content-Disposition: attachment; filename=worldship.$date.csv");
    header("Pragma: no-cache");
    header("Expires: 0");
    echo $header."\n".$data;
}



?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
	<head>
		<title>Orders Pending Shipment</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">

		<script language="Javascript">

			function checkAll(form){
				for(i = 0; i < form.elements.length -1; i++){
					var inType = form.elements[i].type;
					if(inType == "checkbox"){
						if(document.getElementById('selector').innerHTML == "Select All"){
							form.elements[i].checked = true;
						}
						else{
							form.elements[i].checked = false;
						}
					}
				}
				if(document.getElementById('selector').innerHTML == "Select All"){
					document.getElementById('selector').innerHTML = "De-Select";
				}
				else{
					document.getElementById('selector').innerHTML = "Select All";
				}
			}
			function verifyChecked(form){
				isChecked = false;
				for(i = 0; i < form.elements.length -1; i++){
					var inType = form.elements[i].type;
					if(inType == "checkbox" && form.elements[i].checked){
						isChecked = true;
						break;
					}
				}
				if(!isChecked){
					alert("Please check the orders you would like to work with.");
					return false;
				}
				
				return true;;
			}
			
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

		</script>
		<style>
			th{font.size:-1;}
			td{font.size:-1;}
		</style>
	</head>
<body>
<div align="center" style="padding-top:5px;">

<?php if($error):?>
	<p><b><font color="red"><?=$error;?></font></b></p>
<?php elseif($message):?>
	<p><b><font color="blue"><?=$message;?></font></b></p>
<?php endif;?>


<?php if(count($records) > 0):?>

	<form method="post" action="worldship.php" enctype="multipart/form-data">
	
		<h4>UPS Worldship - Orders Pending Shipment</h4>

		<table border="1" cellpadding="2" cellspacing="0" width="100%">
			<tr>
				<th align="left" valign="top">select</th>
				<?php foreach(array_keys($records[0]) as $i=>$fldname): ?>
					<?php $fldname = str_replace('_',' ',$fldname);?>
					<?php $fldname = str_replace('shipaddress','',$fldname);?>
					<th align="left" valign="top" nowrap><?=$fldname;?></th>
				<?php endforeach; ?>
			</tr>
			<?php foreach($records as $index=>$data): ?>
			<tr>
				<td align="center">
					<input type="checkbox" name="checked[<?=$data['order_number'];?>]" value="true">
				</td>
				<?php foreach($data as $key=>$val): ?>
				
					<?php if($key == 'order_number'):?>
						<td align="center">
							<a href="orders.php?order_number=<?=$val;?>&detail=true"><?=$val;?></a>
						</td>
					<?php elseif($val == ""): ?>
						<td>&nbsp;</td>
					<?php else: ?>
						<td><?=$val;?></td>
					<?php endif;?>
				<?php endforeach;?>
			</tr>
			<?php endforeach;?>
			<tr>
				<td nowrap><a href="#" id="selector" onclick="checkAll(document.forms[0]);">Select All</a></td>
				<td colspan="<?=count($data);?>">&nbsp;</td>
			</tr>
		</table>

		<h4>With Selected Orders:</h4>
		<p><input type="submit" name="download" value="Download to Worldship" onClick="return verifyChecked(this.form);"> &nbsp; 
		<input type="submit" name="update" value="Mark as Shipped" onClick="return verifyChecked(this.form);"></p>
		
		<h4>Upload Tracking Numbers:</h4>
		
		<p><input name="tracking_file" type="file" size="40"> &nbsp; <input type="submit" name="submit" value="Upload"></p>
		
		
	</form>

<?php else:?>

	<p>There are no orders to ship</p>

<?php endif;?>


</div>
<p>&nbsp;</p>
</body>
</html>
