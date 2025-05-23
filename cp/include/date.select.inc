<?php
//VersionInfo:Version[3.0.1]

function selectDate (   $sel_d = 0       // selected day
                      , $sel_m = 0       // selected month
                      , $sel_y = 0       // selected year
                      , $var_d = ''     // name for day variable
                      , $var_m = ''     // name for month variable
                      , $var_y = ''     // name for year variable
                      , $min_y = 0       // minimum year
                      , $max_y = 0       // maximum year
                      , $enabled = true  // enable drop-downs?
                    ) {

  // --------------------------------------------------------------------------
  // First of all, set up some sensible defaults

  // Default day is today
  if ($sel_d == 0)
    $sel_d = date('j');

  // Default month is this month
  if ($sel_m == 0)
    $sel_m = date('n');

  // Default year is this year
  if ($sel_y == 0)
    $sel_y = date('Y');

  // Default minimum year is this year
  if ($min_y == 0)
    $min_y = date('Y');

  // Default maximum year is two years ahead
  if ($max_y == 0)
    $max_y = date('Y') + 2;


  // --------------------------------------------------------------------------
  // Start off with the drop-down for Days

  // Start opening the select element
  $days = "<select name=\"$var_d\"";

  // Add disabled attribute if necessary
  if (!$enabled)
    $days .= " disabled=\"disabled\"";

  // Finish opening the select element
  $days .= ">\n";

  // Loop round and create an option element for each day (1 - 31)
  for ($i = 1; $i <= 31; $i++) {

    if($i < 10){
        $i = "0" . $i;
    }

    // Start the option element
    $days .= "\t<option value=\"$i\"";

    // If this is the selected day, add the selected attribute
    if ($i == $sel_d)
      $days .= " selected";

    // Display the value and close the option element
    $days .= ">$i</option>\n";

  }

  // Close the select element
  $days .= "</select>\n";


  // --------------------------------------------------------------------------
  // Now do the drop-down for Months

  // Start opening the select element
  $months = "<select name=\"$var_m\"";

  // Add disabled attribute if necessary
  if (!$enabled)
    $months .= " disabled=\"disabled\"";

  // Finish opening the select element
  $months .= ">\n";

  // Loop round and create an option element for each month (Jan - Dec)
  for ($i = 1; $i <= 12; $i++) {

    if($i < 10){
        $i = "0" . $i;
    }

    // Start the option element
    $months .= "\t<option value=\"$i\"";

    // If this is the selected month, add the selected attribute
    if ($i == $sel_m)
      $months .= " selected";

    // Display the value and close the option element
    //$months .= ">". date('F', mktime(3, 0, 0, $i)). "</option>\n";
    $months .= ">$i</option>\n";

  }

  // Close the select element
  $months .= "</select>\n";


  // --------------------------------------------------------------------------
  // Finally, the drop-down for Years

  // Start opening the select element
  $years = "<select name=\"$var_y\"";

  // Add disabled attribute if necessary
  if (!$enabled)
    $years .= " disabled=\"disabled\"";

  // Finish opening the select element
  $years .= ">\n";

  // Loop round and create an option element for each year ($min_y - $max_y)
  for ($i = $min_y; $i <= $max_y; $i++) {

    // Start the option element
    $years .= "\t<option value=\"$i\"";

    // If this is the selected year, add the selected attribute
    if ($i == $sel_y)
      $years .= " selected";

    // Display the value and close the option element
    $years .= ">" . $i. "</option>\n";

  }

  // Close the select element
  $years .= "</select>\n";

return array($days,$months,$years);
}
?>