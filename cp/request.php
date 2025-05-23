<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "request";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$records = array();

$json = null;
if(!function_exists('json_encode') && file_exists("reports/include/services.json.inc")){
	include_once("reports/include/services.json.inc");
	// create a new instance of Services_JSON
	$json = new Services_JSON();
}

if(isset($_REQUEST['query'])){
	
	$max = 50;
	if(isset($_REQUEST['max'])){
		$max = $_REQUEST['max'];
	}
	
	$param = $_REQUEST['query'];
	$table = null;
	$fields = null;
	
	//logSql($_REQUEST);
	
	if(isset($_REQUEST['table'])){
		$table = $_REQUEST['table'];
	}
	if(isset($_REQUEST['fields'])){
		$fields = $_REQUEST['fields'];
	}
	if($table && $fields){
		
		$fldSet = explode(',',$fields);
		$key = $fldSet[0];
		$len = strlen($param);
		
		if(isset($fldSet[1]) && $fldSet[1] == '*'){
			$fields = '*';	
		}
	}
	
	if($table == "customer_shipping"){

		$sql = "SELECT $fields FROM $table WHERE $key = '$param'";	
		//logSql($sql);
		$records = $_DB->getRecords($sql);

		if(function_exists('json_encode')){
			$result = json_encode($records);
		}
		elseif($json){
			$result = $json->encode($records);
		}
	}
	else{
		if($table == "products" && $fields = 'sku,name,price'){
			// go through discounts
			$sql = "SELECT $fields,categories.catid,categories.category_ids FROM products,product_categories,categories 
					WHERE product_categories.pid = products.pid
					AND categories.catid = product_categories.catid
					AND LEFT(sku,$len) = '$param' LIMIT $max";
			//logSql($sql);
			$records = $_DB->getRecords($sql);
			if(count($records) > 0){
				// check for category or customer discounts
				$discounts = $_Registry->LoadClass("Discounts");
				// check for category or customer discounts
				$discounts->calculateProductDiscounts($records);
				if(!empty($records[0]['qty_price'])){
					$records[0]['price'] = $records[0]['qty_price'];
				}
				//logSql($records);
			}
		}
		else{
			$sql = "SELECT $fields FROM $table WHERE LEFT($key,$len) = '$param'";
			//logSql($sql);
			$records = $_DB->getRecords($sql);	
		}

		$data['records'] = $records;		

		if(function_exists('json_encode')){
			$result = json_encode($data);
		}
		elseif($json){
			$result = $json->encode($data);
		}
	}
}
elseif(isset($_REQUEST['gettax'])){

	global $_Totals;

	include_once("../include/salestax.inc");
	$tax = new Salestax();
	$tax->Country = $_REQUEST['country'];
	$tax->State = $_REQUEST['state'];
	$tax->Zip = $_REQUEST['zip'];
	$_Totals['subtotal'] = $_REQUEST['subtotal'];
	$_Totals['cartTaxableTotal'] = $_REQUEST['subtotal'];
	$_Totals['cartTaxTotal'] = 0;
	
	$salestax = $tax->calculateSalesTax();
	$records[0]['salestax'] = $salestax;
	
	//$results = print_r($_REQUEST, true); 
	//logSql($results);
	
	if(function_exists('json_encode')){
		$result = json_encode($records);
	}
	elseif($json){
		$result = $json->encode($records);
	}
}

function logSql($str){
	if(is_array($str)){
		$str = print_r($str,true);	
	}
	if(!($log = fopen("log.txt", "a"))){
		print("Error: could not append to 'log.txt'\n");
		exit;
	}
	fputs($log,"$str\n");
	fclose($log);	
	
}


header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
header('Last-Modified: ' . gmdate('D, d M Y H:i:s') . 'GMT');
header('Cache-Control: no-cache, must-revalidate');
header('Pragma: no-cache');
header('Content-Type: text/javascript');
print $result;
?>
