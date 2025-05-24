<?php

class favorites{

    public bool $debug = false;
    public $cid = null;

    // ----------------------------------------------------------------
    function __construct(){
		global $_Common,$_DB;
		$md5Cid = empty($_SESSION['cid']) ? null : $_SESSION['cid'];
		if($md5Cid){
			$data = $_DB->getRecord("SELECT cid FROM customers WHERE MD5(cid) = '$md5Cid'");
			if(!empty($data['cid'])){
				$this->cid = $data['cid'];
			}
		}
	}

    // ----------------------------------------------------------------
	function add(){
		
		global $_Common,$_DB;
		
		if($this->cid && !empty($_REQUEST['pid'])){
			$cid = $this->cid;
			$pid = trim($_REQUEST['pid']);
			$count = $_DB->getCount('products',"WHERE `pid` = '$pid'");
			if($count > 0){
				$_DB->execute("INSERT INTO customer_favorites (`cid`,`pid`)VALUES('$cid','$pid')");
				$_Common->sendMessage("The item has been added to your favorites list",true);
			}
			else{
				$_Common->sendMessage("The item could not be added because it was not found in the product list",true);
			}
		}
	}
	
    // ----------------------------------------------------------------
	function delete(){
		
		global $_Common,$_DB;
		
		if($this->cid && !empty($_REQUEST['pid'])){
			$cid = $this->cid;
			$pid = trim($_REQUEST['pid']);
			$count = $_DB->getCount('customer_favorites',"WHERE `cid` = '$cid' AND `pid` = '$pid'");
			if($count > 0){
				$_DB->execute("DELETE FROM customer_favorites WHERE `cid` = '$cid' AND `pid` = '$pid' LIMIT 1");
				$_Common->sendMessage("The item has been removed from your favorites list",true);
			}
			else{
				$_Common->sendMessage("The item is not in your favorites list.",true);
			}
		}
	}
	
    // ----------------------------------------------------------------
	function display(){
		
		global $_Common,$_DB;
		
		if($this->cid){
			$cid = $this->cid;
			$sql = "SELECT * FROM customer_favorites WHERE cid = '$cid' ORDER BY `pid`";
			$data = $_DB->getRecords($sql,'pid');
			if(count($data) > 0){
				$pidList = join(',',array_keys($data));
				$_Common->redirect("products.php?favorites=1&pid=$pidList");
			}
			else{
				$_Common->sendMessage("You do not have any favorites in your list",true);
			}
		}
		else{
			$_Common->sendMessage("You must be logged in to view favorites",true);
		}
	}
	
    // ----------------------------------------------------------------
	function getList(){
		
		global $_Common,$_DB;
		
		if($this->cid){
			$cid = $this->cid;
			$sql = "SELECT * FROM customer_favorites WHERE cid = '$cid' ORDER BY `pid`";
			$data = $_DB->getRecords($sql,'pid');
			if(count($data) > 0){
				return array_keys($data);
			}
			else{
				return array();	
			}
		}
		return array();	
	}
	
	
}
?>
