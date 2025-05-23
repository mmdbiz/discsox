<?php
/**
* Extract filtered results as array from an XML file.
* @author Sam Barnum http://www.360works.com
*/
class XMLSearch {

     /**
     * How filterArray operators are joined.  Must be either AND or OR.
     * If AND, all filterArray items must match a given searchTarget for it to be considered a match.
     */
     var $logicalOperator = "AND";

     /**
     * Name of the XML tag you want to search in.  This will determine the scope of the data that
     * is returned by the search.  Example:  in an XHTML document, choosing 'div' would
     * return an array of the <div> tags that matched your filter.
     * The searchTarget is set in the constructor
     */
     var $searchTarget;

     /**
     * Used to separate multiple tags in the returned array, default value is '|'
     */
     var $delimiter = '|';

     var $limitFrom = 0;
     var $limitTo = 999999999;
     var $_totalMatchesFound = 0;

     /**
     * An associative array representing the filter to be applied to the XML document.
     * Format = $filterArray[$attribute] = array(
     *     "value"          => $value,
     *     "operator"     => $operator
     * );
     * @see setFilter()
     * @see unsetFilter()
     */
     var $filterArray = array();

     /**
     * Count of the number of matches found for the current searchTarget.
     * If the logicalOperator == 'AND', then this number must be greater than the
     * count of the filterArray for the current searchTarget block to be a match.
     */
     var $_matchesForCurrentTarget = null;

     /**
     * Array representing the data for the current searchTarget block being parsed.
     * If the searchTarget is a match, this array will be appended to the $returned array.
     */
     var $_recordBeingParsed;

     /**
     * String containing the tag name being parsed.
     */
     var $_tagBeingParsed;

     /**
     * The xpath for the $_tagBeingParsed within the current $_recordBeingParsed.
     */
     var $_currentXpath = "";
     /**
     * Tracks the last handled xpath, so if an element contains escaped characters the parser concatenates them together.
     */
     var $_previousXpath;

     /**
     * array containing the filtered results
     */
     var $results = array();

	 var $parser;

     /**
     * Constructor for a new XMLSearch object.
     * @param $searchTarget name of the tag being searched.
     * @see searchTarget
     */
     function __construct($searchTarget) {
          $this->searchTarget = $searchTarget;
     }

     /**
     * Set the filter for an tag or attribute.
     * Example: $xmlSearch->setFilter('td', 'like', 'foo');
     * Example: $xmlSearch->setFilter('table@width', 'equals', '64');
     * @param $attribute the tag being parsed.  Use tag@attribute syntax to search in an attribute
     * @param $operator must be one of: 'equals', 'equalsIgnoreCase', 'like', 'greaterThan', 'lessThan'
     * @param $value the value to filter against
     */
     function setFilter($attribute, $operator, $value) {
          $this->filterArray[$attribute] = array("value"=>$value, "operator"=>$operator);
     }

     /**
     * Clear the filter for a tag or attribute.
     * @see setFilter()
     */
     function unsetFilter($attribute) {
          unset($this->filterArray[$attribute]);
     }

     function setLimit($from, $length=null) {
          if ($length === null) { $this->limitFrom = 0; $this->limitTo = $from; }
          else { $this->limitFrom = $from; $this->limitTo = $from + $length; }
     }

     /**
     * Parse the $file XML file and return the matching results.
     * @param $file the path to the XML file being searched.
     * @return Array
     */
     function search($XMLdata) {

		if ($this->validateParameters());
		$this->parser = xml_parser_create();
		xml_set_object($this->parser, $this);
		xml_parser_set_option($this->parser, XML_OPTION_CASE_FOLDING, false);
		xml_set_element_handler($this->parser, "handleStartElement", "handleEndElement");
		xml_set_character_data_handler($this->parser, "handleCharacterData");

		#if (!($fp = fopen($file, "r"))) {
		#    die("XMLSearch could not open XML input file $file");
		#}
		#while ($data = fread($fp, 4096)) {
		#    if (!xml_parse($parser, $data, feof($fp))) {
		#     die(sprintf("XML error in $file: %s at line %d",
		#              xml_error_string(xml_get_error_code($parser)),
		#              xml_get_current_line_number($parser)));
		#    }
		#}

		if (!xml_parse($this->parser, $XMLdata)) {

			print "<pre><H4>XML Parse Error:</H4> \n\nTarget: " . $this->searchTarget . "\n";
			global $DB;
			$DB->showXML($XMLdata);
			exit;
			#die(sprintf("XML error: %s at line %d",
			#         xml_error_string(xml_get_error_code($parser)),
			#         xml_get_current_line_number($parser)));
		}

		xml_parser_free($this->parser);
		return $this->results;
	}

     /**
     * Checks to ensure that the XMLSearch object is correctly configured before beginning searching.
     */
     function validateParameters() {
          if (!$this->searchTarget) die("You must specify a searchTarget");
          //if (count($this->filterArray) == 0) die("You must enter at least one filter");
     }

     function _checkForMatch($path, $value) {
          if (array_key_exists($path, $this->filterArray)) {
               $method = $this->filterArray[$path]['operator']; // determines which equality test method is used
               $toMatch = $this->filterArray[$path]['value'];
               $this->debug( "Checking match for $path: '$toMatch' $method '$value'");
               if ($this->$method($toMatch, $value)) {
                    // found match
                    $this->_matchesForCurrentTarget++;
               } else {
                    // match failed, if using AND it is safe to cancel the rest of this search
                    if ($this->logicalOperator == "AND") $this->_matchesForCurrentTarget = null;
               }
          }
     }

     function debug($text) {
          //echo $text . "<br />\n";
     }

     /*
     ///////////////////////////////////////////////////////////////////////////
     //  CALLBACK FUNCTIONS
     ///////////////////////////////////////////////////////////////////////////
     */
     function handleStartElement($parser, $name, $attrs) {
          if ($this->_matchesForCurrentTarget !== null) {
               if (strlen($this->_currentXpath) > 0) $this->_currentXpath .= '/';
               $this->_currentXpath .= $name;
          }
          if ($name == $this->searchTarget) {
               $this->debug( "---- start SearchTarget: $name ");
               $this->_matchesForCurrentTarget = 0;
               $this->_recordBeingParsed = array();
          }
          $this->_tagBeingParsed = $name;
          if ($this->_matchesForCurrentTarget !== null && is_array($attrs)) {
               // look for attribute matches
               foreach($attrs AS $key=>$value) {
                    $path = $this->_tagBeingParsed . '@' . $key;
                    $this->_checkForMatch($path, $value);
                    $this->_recordBeingParsed[$path] = $value;
               }
          }
     }

     function handleEndElement($parser, $name) {
          $this->_currentXpath = dirname($this->_currentXpath);
          if ($this->_currentXpath == '.') $this->_currentXpath = '';
          if ($name == $this->searchTarget) {
               // see if the current item is a match
               $op = $this->logicalOperator;
               $matches = $this->_matchesForCurrentTarget;



               if ( ($op == 'AND' && $matches >= count($this->filterArray)) || ($op == 'OR' && $matches > 0) ) {
                    $this->_totalMatchesFound++;
                    if ($this->_totalMatchesFound > $this->limitFrom && $this->_totalMatchesFound <= $this->limitTo) {
                        #print "<pre>" . $this->_recordBeingParsed[$name] . "</pre>\n";

$this->results[] = $this->_recordBeingParsed;
/*

                        if(isset($this->_recordBeingParsed['ListID'])){
                            $this->results[$this->_recordBeingParsed['ListID']] = $this->_recordBeingParsed;
                        }
                        else{
                            $this->results[0] = $this->_recordBeingParsed;
                        }
*/
                    }
               }
               // now clear out the matches
               $this->_matchesForCurrentTarget = null;
               $this->_recordBeingParsed = null;
               $this->debug( "--- end searchTarget: $name ");
          }
     }

     function handleCharacterData($parser, $data) {
          if ($this->_matchesForCurrentTarget === null) {
               // not currently in a searchTarget
               return null;
          }
          $data = trim($data);
          if (!$data) return;
          $name = $this->_tagBeingParsed;
          $this->debug( "$name = '$data' ");
          $this->_checkForMatch($name, $data);
          $path = $this->_currentXpath;
          if (isset($this->_recordBeingParsed[$path])) {
               if ($this->_currentXpath != $this->_previousXpath)
                    $this->_recordBeingParsed[$path] .= $this->delimiter;
               $this->_recordBeingParsed[$path] .= $data;
          } else {
               $this->_recordBeingParsed[$path] = $data;
          }
          $this->_previousXpath = $path;
     }
     /*
     ///////////////////////////////////////////////////////////////////////////
     //  SEARCH OPERATORS
     ///////////////////////////////////////////////////////////////////////////
     */
     function equals($v1, $v2) { return $v1 == $v2; }

     function equalsIgnoreCase($v1, $v2) { return strtolower($v1) == strtolower($v2); }

     // FIX!! make this case-insensitive
     function like($v1, $v2) { return strpos($v2, $v1) !== false; }

     function greaterThan($v1, $v2) { return $v1 > $v2; }

     function lessThan($v1, $v2) { return $v1 < $v2; }

}


/* Example usage:
$search = new XMLSearch("TagNameA");
$results = $search->search('xmldata.txt');
echo '<pre>';
print_r($results);
echo '</pre>';
*/
?>
