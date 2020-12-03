/**
 * Generate a random string of the given length from the
 * source alphabet. 
 *
 * Inputs:
 *     length   - output string length
 *     alphabet - character set from which to generate the string
 *
 * Outputs:
 *     none
 *
 * Returns: Randomly generated string of the specified length  
 */
function randomString( length, alphabet )
{
  var arr = [];
  var i;

  for ( i = 0; i < length; i++ )
  {
    arr[i] = alphabet.charAt( Math.random() * alphabet.length );
  }
  return arr.join("");
}

/**
 * Mutate the input string based on a probability table.
 *
 * Inputs:
 *    candidate   - source string to mutate
 *    prob_table  - list of probabilities for each mutation type
 *    alphabet    - character set from which to choose point change characters
 *
 * Outputs:
 *    none
 *
 * Returns: a mutated copy of the candidate string
 */
function mutate( candidate, prob_table, alphabet )
{
  var arr = [];
  var r = 0;
  var w = 0;
  var c;
  
  for ( r = 0; r < candidate.length; r++)
  {
    var event = Math.random() * 100;
    if ( event < prob_table[0] ) // no change
    {
      arr[w++] = candidate[r];
    }
    else if ( event >= prob_table[0] && event < prob_table[1] ) // point change
    {
      arr[w++] = alphabet.charAt( Math.random() * alphabet.length );
    }
    else if ( event >= prob_table[1] && event < prob_table[2] ) // delete
    {
    }
    else if ( event >= prob_table[2] && event < prob_table[3] ) // duplicate
    {
      arr[w++] = candidate[r];
      arr[w++] = candidate[r];
    }
    else // transpose
    {
      if ( r + 1 < candidate.length )
      {
        arr[w++] = candidate[r+1];
      }
      arr[w++] = candidate[r++];     
    }
  }
  return arr.join('');   
}

/**
 * Compute a score for the candidate string by summing 
 * the difference in encodings for each character.  
 * For example, the difference between the strings "ab" and "cd"
 * is 4, since the difference between 'a' and 'c' is 2 and
 * the difference between 'b' and 'd' is 2.  
 *
 * The lower the score, the closer the match.  A score of 0
 * indicates an exact match.
 *
 * Inputs:
 *    candidate - string to be tested
 *    target    - desired target output
 *
 * Outputs:
 *    none
 *
 * Returns: numeric score
 */
function score( candidate, target )
{
  var i = 0;
  var sum = 0;
  
  for ( i = 0; i < candidate.length && i < target.length; i++ )
  {
    sum += Math.abs( candidate.charCodeAt(i) - target.charCodeAt(i) );
  }
  while ( i < candidate.length )
  {
    sum += candidate.charCodeAt(i++);
  }
  while( i < target.length )
  {
    sum += target.charCodeAt(i++);
  }
  return sum;
}

/**
 * Initialize our weasel population by generating a bunch
 * of random strings and scoring them against the target.
 *
 * Inputs:
 *    size      - population size; this does not change over time
 *    target    - string that we are trying to match
 *    strsize   - length of initial population strings
 *    alphabet  - character set from which to generate strings
 *
 * Outputs:
 *    none
 *
 * Returns: array of weasels.
 */
function init_population(size, target, strsize, alphabet)
{
  var pop = new Array();
  var i = 0;
  for ( i = 0; i < size; i++ )
  {
    pop[i] = new Object();   
    pop[i].generation = 1;
    pop[i].string = randomString( strsize, alphabet );
    pop[i].score = score( pop[i].string, target );
  }
  
  return pop;
}
 
/**
 * Validate our input form. The target string must not contain any characters
 * that are not also in the alphabet.  The probabilities must all be between
 * 1 and 97. and they must sum to 100 or less. 
 *
 * Inputs:
 *    none
 *
 * Outputs: 
 *    none
 * 
 * Returns: true if input form passes validation, false otherwise.
 */
function validateInput()
{
  var result = true;
  
  var target = document.getElementById( "target" );
  var alphabet = document.getElementById( "alphabet" );
  var point = document.getElementById( "point" );
  var duplicate = document.getElementById( "duplicate" );
  var del = document.getElementById( "delete" );  
  var xpose = document.getElementById( "transpose" );
  var popsize = document.getElementById( "popsize" );
  var strlen = document.getElementById( "length" );
  
  for ( i = 0; i < target.value.length; i++ )
  {
    if ( alphabet.value.search( target.value[i] ) == -1 )
    {
      result = false;
      break;
    }
  }
  if ( !result )
  {
    alert( "Target contains characters that are not in the alphabet" );
  }
  var probs = point.valueAsNumber + duplicate.valueAsNumber + del.valueAsNumber + xpose.valueAsNumber;
  if ( probs > 100 )
  {
    result = false;
    alert( "Probabilities must sum to 100 or less." );
  }
  if ( point.valueAsNumber < 1 || point.valueAsNumber > 97 || 
       duplicate.valueAsNumber < 1 || duplicate.valueAsNumber > 97 || 
       del.valueAsNumber < 1 || del.valueAsNumber > 97 ||
       xpose.valueAsNumber < 1 || xpose.valueAsNumber > 97 )
  {
    result = false;
    alert( "Probability range must be between 1 and 97" );
  }
  
  if ( point.valueAsNumber > 10 || duplicate.valueAsNumber > 10 || del.valueAsNumber > 10 || xpose.valueAsNumber > 10 )
  {
    result = confirm( "Large change probability values may cause the program to run for an excessive amount of time." );
  }
  
  return result;
}
  
/**
 * Main driver, triggered by pressing OK on the input form. 
 * Validates the input form, then generates and scores the
 * initial population.  Population is sorted by score in 
 * ascending order.  
 *
 * While there is no match to the target string, discard the 90%
 * of the population with the highest scores, rebuild
 * the population from the remaining 10% by copying them
 * with mutations, score the new strings against the target,
 * and sort the population by score.   
 *
 * Each time the string with the lowest score changes, display it
 * to the output table.
 * 
 * Inputs:
 *    none
 *
 * Outputs:
 *    none
 *
 * Returns: none
 */
function doWeasel()
{
  if ( !validateInput() )
  {
    return;
  }
  
  var outputarea = document.getElementById( "outputarea" );

  var popsize = document.getElementById( "size" );
  var target = document.getElementById( "target" );
  var alphabet = document.getElementById( "alphabet" );
  var strsize = document.getElementById( "length" );
  
  var table = document.getElementById( "outputTable" );
  if ( table != undefined )
  {
    outputarea.removeChild( table );
  }
  
  var prob_table = [96, 97, 98, 99];
  var param = document.getElementById ( "transpose" );
  prob_table[3] = 100 - param.value;
  param = document.getElementById( "delete" );
  prob_table[2] = prob_table[3] - param.value;
  param = document.getElementById( "duplicate" );
  prob_table[1] = prob_table[2] - param.value;
  param = document.getElementById ("point" );
  prob_table[0] = prob_table[1] - param.value;
  
  
  var population = init_population( popsize.value, target.value, strsize.value, alphabet.value );
  population.sort( function(a,b) { return a.score - b.score; });
  
  var table = document.createElement( "table" );
  table.setAttribute( "id", "outputTable" );
  var head = table.createTHead();
  var row = head.insertRow();
  var cell = row.insertCell();
  cell.innerHTML = "Generation";
  cell = row.insertCell();
  cell.innerHTML = "Score";
  cell = row.insertCell();
  cell.innerHTML = "String";
  table.appendChild( head );
  
  row = table.insertRow();
  
  cell = row.insertCell();
  cell.innerHTML = population[0].generation;
  cell = row.insertCell();
  cell.innerHTML = population[0].score;
  cell = row.insertCell();
  cell.innerHTML = population[0].string;
  var p;
  var g = 1;
  var breeders = Math.floor( population.length / 10 );
  
  var best_fit = population[0].string;
  
  while( population[0].score > 0 )
  {
    g++;
    for ( p = breeders; p < population.length; p++ )
    {
      population[p].generation = g;
      population[p].string = mutate( population[p%breeders].string, prob_table, alphabet.value );
      population[p].score = score( target.value, population[p].string );
    }
    population.sort( function(a,b) { return a.score - b.score; } );
    
    if ( population[0].string != best_fit )
    {
      row = table.insertRow();
      cell = row.insertCell();
      cell.innerHTML = population[0].generation;
      cell = row.insertCell();
      cell.innerHTML = population[0].score;
      cell = row.insertCell();
      cell.innerHTML = population[0].string;
      best_fit = population[0].string;
    }
  }  
  outputarea.append( table );
}
