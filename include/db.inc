<?php
// -----------------------------------------------------------------------
//         Class: Db
//       Purpose: This class controls the database functionality for
//                MySQL databases.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Db{

    var $cnx;
    var $debug = false;
    var $error = null;
    var $previousNextLinks;
    var $queryVars = array();
    var $serverVersion = NULL;


    function __construct(){
        // Connect to the database
        $this->cnx = $this->connect();
    }

    // ----------------------------------------------------------------
    // Function name
    //      close
    // General description
    //      This function closes the connection to the database
    // Inputs
    //      None
    // Outputs
    //      Status of function: success (true) / failure (false)
    // Modified Date
    //      February 24, 2004
    //
    function close(){
        return @mysqli_close($this->cnx);
    }


    // ----------------------------------------------------------------
    // Function name
    //      connect
    // General description
    //      This function makes a connection to the database
    // Inputs
    //      None
    // Outputs
    //      Link Identifier
    // Modified Date
    //      February 24, 2004
    //
    function connect(){

        global $_CF;

        $link_identifier = @mysqli_connect(DB_HOST, DB_USERNAME, DB_PASSWORD);
        if(!$link_identifier){
            die(mysqli_errno($link_identifier) . ": " . mysqli_error($link_identifier));
        }

        $result = mysqli_select_db($link_identifier, DB_NAME );
        if(!$result){
            die(mysqli_errno($link_identifier) . ": " . mysqli_error($link_identifier));
        }

        // get the version and set it so we can compare versions
        @list($this->serverVersion,$os) = explode("-",mysqli_get_server_info($link_identifier));
        $values = explode("\.",$this->serverVersion);
        $this->serverVersion = "";
        foreach($values as $strNum){
            if($strNum < 10){
                $strNum = "0".$strNum;
            }
            $this->serverVersion .= $strNum;
        }
        $this->serverVersion = intval($this->serverVersion);

        return $link_identifier;

    }

    // ----------------------------------------------------------------
    // Function name
    //      execute
    // General description
    //      This function executes the query (SQL Statement)
    // Inputs
    //      $link_identifier = Link Identifier
    //      $sql = Query String (SQL Statement)
    // Outputs
    //      Result (usually a recordset on success)
    // Modified Date
    //      February 27, 2004
    //
    function execute($sql, $link = "", $display_errors = false){
	
		//marcello disable deprecated messages
		//error_reporting(E_ALL & ~E_DEPRECATED);
		error_reporting(E_ALL);
		//error_reporting(E_ALL & ~E_NOTICE);

		global $_Common;
		
		if($this->debug){
			$_Common->debugPrint($sql);
			flush();
		}

        if(is_null($link) || trim($link) == ""){
            $link = $this->cnx;
        }
        // Strip out "*" in delete
        if(preg_match("/delete \*/i", $sql)){
            $sql = preg_replace("/delete \*/i", "DELETE", $sql);
        }

        $conn = @mysqli_query($link, $sql);
        if(!$conn && mysqli_errno($link) > 0){
			if($display_errors){
				print "MySQL Error: " . mysqli_errno($link) . ": " . mysqli_error($link) . "<br>$sql\n";
			}
			$this->error = mysqli_errno($link);
			
			// bad table, try to repair it
			if(mysqli_errno($link) == 144 || strstr(mysqli_error($link),'errno: 144')){
				$this->repairTables();
			}
		}
		
		if($this->error && $display_errors){
			$_Common->debugPrint(mysqli_error($link));
		}
		
        return $conn;
    }


    // ----------------------------------------------------------------
    // Function name
    //      fetchrow
    // General description
    //      This function returns a row from the database
    // Inputs
    //      $rs = Recordset
    //      $type = Optional.  Array type to return (Associative or
    //              Numeric).  Default is Numeric
    // Outputs
    //      The row
    // Modified Date
    //      February 24, 2004
    //
    function fetchrow($rs, $type = ''){

        if($type == 'ASSOC'){
            $type = MYSQLI_ASSOC;
        }
        else{
            $type = MYSQLI_NUM;
        }
        $row = array();
        if($rows = @mysqli_fetch_array($rs, $type)){
			$row = array_change_key_case($rows,CASE_LOWER);
        }
        return($row);
    }


    // -------------------------------------------------------------------
    // Function name
    //      free_result
    // General description
    //      This function frees up the memory used by the recordset
    // Inputs
    //      $link_identifier = Link Identifier
    // Outputs
    //      True on success, false failure
    // Modified Date
    //      February 24, 2004
    //
    function free_result($link_identifier){
        return @mysqli_free_result($link_identifier);
    }

    // ----------------------------------------------------------------
    // Function name
    //   getFieldProperties
    // General description
    //   This function gets the properties for the fields in the
    //   specified table

    function getFieldProperties($table){

        $RS = $this->execute("SHOW FIELDS FROM $table");
        $fields = array();
        while($row = $this->fetchrow($RS)){
            $fields[strtolower($row[0])] = $row;
        }

    return $fields;
    }

    // ----------------------------------------------------------------
	function getDefaultValues($key,$value = NULL,$makeSelect = false,$fldname = null){
		
		global $_Common;
		global $fldProperties;

		//$_Common->debugPrint($fldProperties,$key);

		$defaults = array();
		if(!empty($fldProperties[$key][1]) && strstr($fldProperties[$key][1],"enum")){
			$defaultStr = preg_match("|\((.*)\)|",$fldProperties[$key][1],$match);
			$match[1] = preg_replace("|\'|","",$match[1]);
			$defaults = explode(",",$match[1]);
			if(count($defaults) > 0){
				//$_Common->debugPrint($defaults,$key);
				
				if($value == NULL && !empty($fldProperties[$key][4])){
					$value = $fldProperties[$key][4];
				}
				if($makeSelect || isset($_REQUEST['edit']) || isset($_REQUEST['modify']) || isset($_REQUEST['add']) || isset($_REQUEST['insert']) || isset($_REQUEST['update'])){
					
					if($fldname){
						list($select,$selected) = $_Common->makeSelectBox($fldname,$defaults,$defaults,$value,false);
					}
					else{
						list($select,$selected) = $_Common->makeSelectBox($key,$defaults,$defaults,$value,false);
					}
					$value = $select;
				}
				else{
					if(isset($defaults[0]) && $value == NULL){
						$value = $defaults[0];
					}	
				}
			}
		}
		elseif($value == NULL && !empty($fldProperties[$key][4])){
			if($key == "customer_number"){
				$sql = "SELECT MAX(customer_number) as lastid FROM customers";
				$row = $this->fetchrow($this->execute($sql),"ASSOC");
				if(isset($row['lastid']) && $row['lastid'] != 0){
					$value = $row['lastid'] + 1;
				}
				else{
					$value = $fldProperties[$key][4];
				}
			}
			else{
				$value = $fldProperties[$key][4];
				if($value == "0000-00-00"){
					$value = "";
				}
			}
		}
		//$_Common->debugPrint($value);
		return $value;
	}


    // ----------------------------------------------------------------
    // Function name
    //   getTableComments
    // General description
    //   This function gets the comments for a specified table

    function getTableComments($table){
		global $_Common;
		$comments = NULL;
        $RS = $this->execute("SHOW TABLE STATUS LIKE '$table'");
        $row = $this->fetchrow($RS,'ASSOC');
        $this->free_result($RS);
        if(!empty($row['comment'])){
	        $comments = $row['comment'];
	    }
    return $comments;
    }

    // ----------------------------------------------------------------
    // Function name
    //      numrows
    // General description
    //      This function gets the number of rows in the specified table
    // Inputs
    //      $result = Result (Usually returned from an execute statement)
    // Outputs
    //      Number of rows
    // Modified Date
    //      February 24, 2004
    //
    function numrows($result){
        return @mysqli_num_rows($result);
    }

    // -------------------------------------------------------------------
    function mysql_table_exists($table){
        $link = $this->cnx;
        $exists = @mysqli_query($link, "SELECT 1 FROM `$table` LIMIT 0");
        if($exists) return true;
        return false;
    }

    // ----------------------------------------------------------------
    // Function name
    //      lastInertID
    // General description
    //      This function gets the ID number of the last insert
    // Inputs
    //      $result = Result (Usually returned from an execute statement)
    // Outputs
    //      Number of rows
    // Modified Date
    //      February 24, 2004
    //
    function lastInsertID($link=''){
		if(trim($link) == ''){
            $link = $this->cnx;
        }
        return mysqli_insert_id($link);
    }

    // ----------------------------------------------------------------
	function getInsertID($table,$field){
		
		global $_Common;
		
		$lastid = $this->lastInsertID();
		
		if($lastid == 0){
			$sql = "SELECT MAX($field) as lastid FROM $table";
			$row = $this->fetchrow($this->execute($sql),"ASSOC");
			if(isset($row['lastid']) && $row['lastid'] != 0){
				$lastid = $row['lastid'];
			}
		}
		if($lastid == 0){
			print "<b>ERROR:</b> Could not get last insert id from mysql";
			exit;
		}
		return $lastid;
	}



    // -------------------------------------------------------------------
    // Function name
    //   getCount
    // General description
    //   gets the count of the resulting sql/where statment
    // Inputs
    //   table/where
    // Outputs
    //   count
    // Modified Date
    //   May 14, 2004
    //

    function getCount($table, $where = ""){

		global $_Common;

        $sql = "SELECT COUNT(*) as total FROM $table $where";

		//$_Common->debugPrint($sql);

        // Run the query
        $rs = $this->execute($sql);
        if(!$rs){
             print "SQL statement failed near line " . __LINE__ . "in " . basename(__FILE__) . ".<br>\n";
             $this->close();
             exit;
        }

        while($row = $this->fetchrow($rs,"ASSOC")){
            $count = $row['total'];
            break;
        }
		$this->free_result($rs);
		
    return $count;
    }


    // -------------------------------------------------------------------
    function makeAddFields($fieldProperties, $idFld, $data){

		global $_Common;

        $fields = "";
        $values = "";

	
        foreach($fieldProperties as $fldName=>$props){
            if($fldName == $idFld){
                continue;
            }
            if(isset($data[$fldName])){
				
				if(is_array($data[$fldName])){
					continue;
				}
				
                $fldValue = trim($data[$fldName]);
                $dbFldName = $props[0];
                $fldValue = preg_replace("|\'|","`",$fldValue);

				if(trim($fldValue) == "NA"){
					$fldValue = NULL;
				}

				if($dbFldName == "card_number" || $dbFldName == "verification_number" || strtolower($dbFldName) == "cvv2"){
					if($fldValue != ""){
						//$_Common->debugPrint("$dbFldName = $fldValue","Incoming");
						
						$fldValue = $this->EncryptValue($fldValue);
						
						//$_Common->debugPrint("$dbFldName = $fldValue","encrypted");
					}
				}
                if($fields == ""){
                    $fields .= "`$dbFldName`";
					if(stristr($fldValue,"ENCODE(")||stristr($fldValue,"AES_ENCRYPT")){                    
						$values .= $fldValue;
					}
					else{
						$values .= "'$fldValue'";
					}
                }
                else{
                    $fields .= ", `$dbFldName`";
					if(stristr($fldValue,"ENCODE(")||stristr($fldValue,"AES_ENCRYPT")){                    
						$values .= ", $fldValue";
					}
					else{                    
						$values .= ", '$fldValue'";
					}
                }
            }
        }
        return array($fields, $values);
    }

    // -------------------------------------------------------------------
    function makeUpdateFields($fieldProperties,$idFld,$data){

        $values = "";

        foreach($fieldProperties as $fldName=>$props){
            if($fldName == $idFld){
                continue;
            }
            if(isset($data[$fldName]) && !is_array($data[$fldName])){
                $fldValue = trim($data[$fldName]);
                $dbFldName = $props[0];

                $fldValue = preg_replace("|\'|","`",$fldValue);

				if($dbFldName == "card_number" || $dbFldName == "verification_number" || strtolower($dbFldName) == "cvv2"){
					if($fldValue != ""){
						$fldValue = $this->EncryptValue($fldValue);
					}
				}

                if($values == ""){
					if(stristr($fldValue,"ENCODE(")||stristr($fldValue,"AES_ENCRYPT")){                    
						$values .= "`$dbFldName` = $fldValue";
					}
					else{
						$values .= "`$dbFldName` = '$fldValue'";
					}					
                }
                else{
					if(stristr($fldValue,"ENCODE(")||stristr($fldValue,"AES_ENCRYPT")){                    
						$values .= ", `$dbFldName` = $fldValue";
					}
					else{
						$values .= ", `$dbFldName` = '$fldValue'";
					}							
                }
            }
        }

    return $values;
    }

	//---------------------------------------------------------------
	function EncryptValue($value){

		global $_CF;
		global $_Common;

		$crypt_key = $_CF['encryption']['security_phrase'];
		
		if(!empty($crypt_key)){
			// Encrypt Credit Card Fields
			if($this->serverVersion >= 40002){
				$value = "AES_ENCRYPT('$value','$crypt_key')";
			}
			else{
				$value = "ENCODE('$value','$crypt_key')";
			}
		}		
		else{
			$errorTitle = "Security Phrase Missing";
			$errorMessage = "Configuration Error!";
			$errorMessage .= "<p>A security phrase has not been enter into the store encryption configuration.";
			$errorMessage .= "<p>Please login to the store control panel and select the configuration/encryption section to correct this.";
			$_Common->printErrorMessage($errorTitle,$errorMessage);
		}


	return $value;
	}

	// -------------------------------------------------------------------
	// gets a single record
    function getRecord($sql){
		global $_Common;
		$rs = $this->execute($sql);
		$row = $this->fetchrow($rs, "ASSOC");
		$this->free_result($rs);
        return $_Common->array_trim($row);
	}

    // -------------------------------------------------------------------
    // Function name
    //      getRecords
    // General description
    //      This runs the SQL and returns the records.  Usually used with
    //      SELECT
    // Inputs
    //      $sql = The SQL statement to run
    //		$keyName = Field whos value shoudl be used as the array key
    // Outputs
    //      Array of records found
    // Modified Date
    //      09-30-2004
    //
    function getRecords($sql,$keyName=null){

		global $_Common;

        $rs = $this->execute($sql);
		
        $rows = array();
        if(is_null($keyName)){
			while($row = $this->fetchrow($rs, "ASSOC")){
				$rows[] = $_Common->array_trim($row);
			}	
		}else{
			while($row = $this->fetchrow($rs, "ASSOC")){
				$rows[trim($row[$keyName])] = $_Common->array_trim($row);
			}	
		}
        
        $this->free_result($rs);
        
        return $rows;
    }


	//---------------------------------------------------------------
	function escape_string(&$string){
		
		if(version_compare(phpversion(),"4.3.0") == "-1"){
			return mysql_escape_string($string);
		}
		elseif($this->cnx){
			return mysqli_real_escape_string($this->cnx, $string);
		}
		else{
			return false;
		}
	}


	// --------------------------------------------------------------
	function getLimits($recordCount,$maxRows,$script = "display.php"){

		$detail = false;
		$hits = 0;
		
        if(!empty($_REQUEST['detail'])){
			$detail = true;
            $maxRows = 1;
        }
		if(!$detail && !empty($_REQUEST['per_page'])){
			if(intval($_REQUEST['per_page']) > 0){
				$maxRows = intval($_REQUEST['per_page']);
				$this->queryVars['per_page'] = $maxRows;
			}
		}
		if(!$detail && !empty($_REQUEST['max'])){
			if(intval($_REQUEST['max']) > 0){
				$maxRows = intval($_REQUEST['max']);
				$this->queryVars['max'] = $maxRows;
			}
		}

		if(!empty($_REQUEST['hits']) && intval($_REQUEST['hits']) > 0){
			$hits = intval($_REQUEST['hits']);
			$this->queryVars['hits'] = $hits;
		}
		if($detail){
            $hits = 0;
		}

		$start = $hits + 1;
		$end = $maxRows + $hits;
		if($end > $recordCount){
			$end = $recordCount;
		}

		// previous/next links
		$this->createPreviousNextLinks($hits,$recordCount,$maxRows,$script);

		// Set SQL Limit
		if($detail){
			$limit = " LIMIT 1";
		}
		else{
			$limit = " LIMIT $hits,$maxRows";
		}

	return array($start,$end,$limit);
	}

    // -------------------------------------------------------------------
    function createPreviousNextLinks($hits,$TotalRecords,$maxRows,$script = "display.php"){

        if($hits < 0){
            $hits = 0;
        }

        $linkCount = ($TotalRecords / $maxRows);
        $CurrentPage = ($hits / $maxRows) + 1;
        $CurrentPage = ceil($CurrentPage);

        $TotalResultPages = intval($TotalRecords / $maxRows);
        if(($TotalRecords % $maxRows) != 0) {
             $TotalResultPages += 1;
        }

        $LoPage = $CurrentPage - 1;
        $HiPage = $CurrentPage + 8;

        if($LoPage<1) {
             $LoPage =1;
             if($TotalResultPages > 10) {
               $HiPage = 10;
             }
             else {
               $HiPage = $TotalResultPages;
             }
        }

        if($HiPage > $TotalResultPages) {
             $HiPage = $TotalResultPages;
             $LoPage = $HiPage - 9;
             if($LoPage < 1) {
               $LoPage = 1;
             }
        }

        $PreviousNextLinks = "";
        $queryString = "";

		foreach($this->queryVars as $key=>$val){
            if($key == "hits"){
                continue;
            }
			if($queryString == ""){
				$queryString = "$key=$val";
			}
			else{
				$queryString .= "&amp;$key=$val";
			}
		}

		if($TotalResultPages > 1) {

			if(($HiPage - 1) >= 10 || $CurrentPage > 1){
				$Image = "<img src=\"images/nav/arrow_first_on.gif\" border=\"0\" alt=\"first\">";
                $PreviousNextLinks = "<a href=\"$script?$queryString\">$Image</A>&nbsp;";
            }
			else{
				$PreviousNextLinks = "<img src=\"images/nav/arrow_first_off.gif\" border=\"0\" alt=\"first\">&nbsp;";
			}

            if($CurrentPage > 1){
                $pHits = ($hits - $maxRows);
                $pImage = "<img src=\"images/nav/arrow_previous_on.gif\" border=\"0\" alt=\"previous\">";
                $PreviousNextLinks .= "<a href=\"$script?$queryString&amp;hits=$pHits\">$pImage</A>&nbsp;&nbsp;";
            }
			else{
				$PreviousNextLinks .= "&nbsp;<img src=\"images/nav/arrow_previous_off.gif\" border=\"0\" alt=\"previous\">&nbsp;";
			}

			$PreviousNextLinks .= "&nbsp;";

			$formHits = $hits;
			$highCount = 0;

            for($i = $LoPage; $i <= $HiPage; $i++){
				$hits = (($i - 1) * $maxRows) - $maxRows;
				if($hits < 0){
					$hits = 0;
				}

				$nHits = ($hits + $maxRows);
				$highCount = $nHits;

				if($i == $CurrentPage){
					$PreviousNextLinks .= "&nbsp;<b>$i</b>&nbsp;&nbsp;";
				}
				else{
					if($i == 1){
						$PreviousNextLinks .=
							"&nbsp;<a href=\"$script?$queryString&amp;hits=0\">$i</A>&nbsp;&nbsp;";
					}
					elseif($i > $CurrentPage){
						$PreviousNextLinks .=
							"&nbsp;<a href=\"$script?$queryString&amp;hits=$nHits\">$i</A>&nbsp;&nbsp;";
					}
					else{
						$PreviousNextLinks .=
							"&nbsp;<a href=\"$script?$queryString&amp;hits=$nHits\">$i</A>&nbsp;&nbsp;";
					}
				}
            }
            if($CurrentPage != $TotalResultPages){
                $nHits = ($formHits + $maxRows);
                $nImage = "<img src=\"images/nav/arrow_next_on.gif\" border=\"0\" alt=\"next\">";
                $PreviousNextLinks .= "&nbsp;&nbsp;<a href=\"$script?$queryString&amp;hits=$nHits\">$nImage</a>";
            }
			else{
				$PreviousNextLinks .= "&nbsp;<img src=\"images/nav/arrow_next_off.gif\" border=\"0\" alt=\"next\">&nbsp;";
			}
            if($HiPage < $TotalResultPages){
                $nHits = (($TotalResultPages - 1) * $maxRows);
                $lImage = "<img src=\"images/nav/arrow_last_on.gif\" border=\"0\" alt=\"last\">";
                $PreviousNextLinks .= "&nbsp;<a href=\"$script?$queryString&amp;hits=$nHits\">$lImage</a>&nbsp;&nbsp;";
            }
            elseif($CurrentPage != $TotalResultPages){
                $lImage = "<img src=\"images/nav/arrow_last_on.gif\" border=\"0\" alt=\"last\">";
                $PreviousNextLinks .= "&nbsp;&nbsp;<a href=\"$script?$queryString&amp;hits=$highCount\">$lImage</a>";
			}
			else{
				$PreviousNextLinks .= "&nbsp;<img src=\"images/nav/arrow_last_off.gif\" border=\"0\" alt=\"last\">&nbsp;";
			}
        }

		$this->previousNextLinks = $PreviousNextLinks;
    }

	// -------------------------------------------------------------------
	function datadump($table,$backupFile) {

		global $cnx;

		$rs = execute("SELECT * FROM $table");

		if(mysqli_num_rows($rs) > 0){
			
			if(!($handle = fopen($backupFile, "w"))){
				print("Error: could not write to '$backupFile'\n");
				exit;
			}

			while($flds = mysqli_fetch_array($rs, MYSQL_ASSOC)){
				
				// do cleanup just in case
				$vals = array_values($flds);
				foreach($vals as $i=>$val){
					$val = str_replace("\n"," ",$val);
					$val = str_replace("\r","",$val);
					$vals[$i] = mysql_real_escape_string($val);
				}
				
				$keys   = "`" . join('`,`',array_keys($flds)) . "`";
				$values = "'" . join("','",$vals) . "'";

				$row = "INSERT INTO " . $table . " ($keys) VALUES ($values)\n";
				fputs($handle,$row);
			}
			
			fclose($handle);
		}
	}

	// -------------------------------------------------------------------
	function repairTables(){
		
		global $_Common;
		
		$sql = "SHOW TABLES FROM " . DB_NAME;
		$result = @mysqli_query($this->cnx, $sql);
		if(!$result) {
			print "DB Error, could not list tables<br />";
			print 'MySQL Error: ' . mysql_error();
			exit;
		}
		while ($row = @mysqli_fetch_row($result)) {
			$table = $row[0];
			$repaired = @mysqli_query($this->cnx, "REPAIR TABLE $table");
			if(!$repaired) {
				print "DB Error: " . mysql_error() . "<br />";
				print "Could not repair table: $table.<br />";
			}
			@mysqli_free_result($repaired);
		}
		@mysqli_free_result($result);
	}


} // End of class Mysql_Db

?>