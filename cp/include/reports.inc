<?php
//VersionInfo:Version[3.0.1]

class reports{

    var $debug = false;

	///<summary>
	///Looks in the reports directory for report files to run.<summary>
	///<returns>
	///array of report names and filenames</returns>
    function getReportList(){

		$cwd = getcwd();
        $path = "$cwd/reports";
        $dirlist = array();
        $handle = opendir($path);
        while ($file = readdir($handle)) {
            if(is_file("$path/$file") && preg_match("/^report./i", $file)) {
                //$names = split("\.",$file);
                $names = preg_split("/\./",$file); //marcello fix deprecated
                //lose the leading 'report.' and the extension
                array_pop($names);
                array_shift($names);
                $fileName = join(".", $names);
                $reportName = join(" ", $names);
                $dirlist[$fileName] = ucwords($reportName);
            }
        }
        return $dirlist;
    }

	///<summary>
	///Loads the requested report and runs it </summary>
    function runReport(){
        global $_Common;
        $reportName = strtolower($_REQUEST['report']);
		$fullReportName = 'reports/'.'report.'.$reportName.'.inc';
		$cwd = getcwd();
		$fullReportName = $cwd.'/'.$fullReportName;

$_Common->debugPrint($fullReportName);

        if(file_exists($fullReportName)){
            include $fullReportName;
        }
        else{
            $_Common->printErrorMessage("Report Not Found","The $reportName report could not be found");
        }
    }


} // End of class reports

?>