//  (C) Copyright Ben Bear, Herve Bronnimann 2007.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
 Revision history:
Nov 13, 2007:  Incorporation of boost-devel comments (Jens Seidel, Ben Bear and myself)
Nov 11, 2007:  Rewrite of Ben Bear's Gacap
*/

// Reference:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2639.pdf

#ifndef BOOST_ALGORITHM_COMBINATION_HPP
#define BOOST_ALGORITHM_COMBINATION_HPP

#include <algorithm>

namespace boost {

namespace detail {

template<class BidirectionalIterator> 
  bool next_combination(BidirectionalIterator first1,
                        BidirectionalIterator last1,
                        BidirectionalIterator first2,
                        BidirectionalIterator last2)
  {
      if ((first1 == last1) || (first2 == last2)) {
          return false;
      }
      
      BidirectionalIterator m1 = last1;
      BidirectionalIterator m2 = last2; --m2;
      
      // Find first m1 not less than *m2 (i.e., lower_bound(first1, last1, *m2)).
      // Actually, this loop stops at one position before that, except perhaps
      // if m1 == first1 (in which case one must compare *first1 with *m2).
      while (--m1 != first1 && !(*m1 < *m2)) {
      }
      
      // Test if all elements in [first1, last1) not less than *m2.
      bool result = (m1 == first1) && !(*first1 < *m2);
      
      if (!result) {

          // Find first first2 greater than *m1 (since *m1 < *m2, we know it
          // can't pass m2 and there's no need to test m2).
          while (first2 != m2 && !(*m1 < *first2)) {
              ++first2;
          }
          
          first1 = m1;
          std::iter_swap (first1, first2);
          ++first1;
          ++first2;
      }
      
      // Merge [first1, last1) with [first2, last2), given that the rest of the
      // original ranges are sorted and compare appropriately.
      if ((first1 != last1) && (first2 != last2)) {      
          for (m1 = last1, m2 = first2;  (m1 != first1) && (m2 != last2); ++m2) {
              std::iter_swap (--m1, m2);
          }
          
          std::reverse (first1, m1);
          std::reverse (first1, last1);
          
          std::reverse (m2, last2);
          std::reverse (first2, last2);
      }
      
      return !result;
  }
    
template<class BidirectionalIterator, class Compare> 
  bool next_combination(BidirectionalIterator first1,
                        BidirectionalIterator last1,
                        BidirectionalIterator first2,
                        BidirectionalIterator last2, Compare comp)
  {
      if ((first1 == last1) || (first2 == last2)) {
          return false;
      }
      
      BidirectionalIterator m1 = last1;
      BidirectionalIterator m2 = last2; --m2;
      
      while (--m1 != first1 && !comp(*m1, *m2)) {
      }
      
      bool result = (m1 == first1) && !comp(*first1, *m2);
      
      if (!result) {
          
          while (first2 != m2 && !comp(*m1, *first2)) {
              ++first2;
          }

          first1 = m1; 
          std::iter_swap (first1, first2);
          ++first1;
          ++first2;
      }
      
      if ((first1 != last1) && (first2 != last2)) {      
          for (m1 = last1, m2 = first2;  (m1 != first1) && (m2 != last2); ++m2) {
              std::iter_swap (--m1, m2);
          }
          
          std::reverse (first1, m1);
          std::reverse (first1, last1);
          
          std::reverse (m2, last2);
          std::reverse (first2, last2);
      }
      
      return !result;
  }
  
}  // namespace detail

/* PROPOSED STANDARD EXTENSIONS:
 *
 * template<class BidirectionalIterator> 
 *   bool next_partial_permutation(BidirectionalIterator first,
 *                                 BidirectionalIterator middle,
 *                                 BidirectionalIterator last); 
 *
 * template<class BidirectionalIterator, class Compare> 
 *   bool next_partial_permutation(BidirectionalIterator first,
 *                                 BidirectionalIterator middle,
 *                                 BidirectionalIterator last, Compare comp); 
 */
 
template <class BidirectionalIterator>
  bool next_partial_permutation(BidirectionalIterator first,
                                BidirectionalIterator middle,
                                BidirectionalIterator last)
{
  reverse (middle, last);
  return next_permutation(first, last);
}

template<class BidirectionalIterator, class Compare> 
  bool next_partial_permutation(BidirectionalIterator first,
                                BidirectionalIterator middle,
                                BidirectionalIterator last, Compare comp)
{
  reverse (middle, last);
  return next_permutation(first, last, comp);
}

/* PROPOSED STANDARD EXTENSIONS:
 *
 * template<class BidirectionalIterator> 
 *   bool prev_partial_permutation(BidirectionalIterator first,
 *                                 BidirectionalIterator middle,
 *                                 BidirectionalIterator last); 
 *
 * template<class BidirectionalIterator, class Compare> 
 *   bool prev_partial_permutation(BidirectionalIterator first,
 *                                 BidirectionalIterator middle,
 *                                 BidirectionalIterator last, Compare comp); 
 */
 
template<class BidirectionalIterator> 
  bool prev_partial_permutation(BidirectionalIterator first,
                                BidirectionalIterator middle,
                                BidirectionalIterator last)
{
  bool result = prev_permutation(first, last);
  reverse (middle, last);
  return result;
}


template<class BidirectionalIterator, class Compare> 
  bool prev_partial_permutation(BidirectionalIterator first,
                                BidirectionalIterator middle,
                                BidirectionalIterator last, Compare comp)
{
  bool result = prev_permutation(first, last);
  reverse (middle, last);
  return result;
}

/* PROPOSED STANDARD EXTENSIONS:
 *
 * template<class BidirectionalIterator> 
 *   bool next_combination(BidirectionalIterator first,
 *                         BidirectionalIterator middle,
 *                         BidirectionalIterator last); 
 *
 * template<class BidirectionalIterator, class Compare> 
 *   bool next_combination(BidirectionalIterator first,
 *                         BidirectionalIterator middle,
 *                         BidirectionalIterator last, Compare comp); 
 */

template<class BidirectionalIterator> 
  bool next_combination(BidirectionalIterator first,
                        BidirectionalIterator middle,
                        BidirectionalIterator last)
  {
    return detail::next_combination(first, middle, middle, last);
  }
    
template<class BidirectionalIterator, class Compare> 
  bool next_combination(BidirectionalIterator first,
                        BidirectionalIterator middle,
                        BidirectionalIterator last, Compare comp)
  {
      return detail::next_combination(first, middle, middle, last, comp);
  }
 
/* PROPOSED STANDARD EXTENSIONS:
 *
 * template<class BidirectionalIterator> 
 *   bool prev_combination(BidirectionalIterator first,
 *                         BidirectionalIterator middle,
 *                         BidirectionalIterator last); 
 *
 * template<class BidirectionalIterator, class Compare> 
 *   bool prev_combination(BidirectionalIterator first,
 *                         BidirectionalIterator middle,
 *                         BidirectionalIterator last, Compare comp); 
 */
 
template<class BidirectionalIterator>
  inline
  bool prev_combination(BidirectionalIterator first,
                        BidirectionalIterator middle,
                        BidirectionalIterator last)
  {
    return detail::next_combination(middle, last, first, middle);
  }
  
template<class BidirectionalIterator, class Compare> 
  inline
  bool prev_combination(BidirectionalIterator first,
                        BidirectionalIterator middle,
                        BidirectionalIterator last, Compare comp)
  {
    return detail::next_combination(middle, last, first, middle, comp);
  }
  
/* PROPOSED STANDARD EXTENSION:
 *
 * template<class BidirectionalIterator, class T> 
 *   bool next_mapping(BidirectionalIterator first,
 *                     BidirectionalIterator last,
 *                     T first_value, T last_value); 
 *
 * template<class BidirectionalIterator, class T, class Incrementor> 
 *   bool next_mapping(BidirectionalIterator first,
 *                     BidirectionalIterator last,
 *                     T first_value, T last_value, Incrementor increment); 
*/
 
template <class BidirectionalIterator, class T>
  bool
  next_mapping(BidirectionalIterator first,
               BidirectionalIterator last,
               T first_value, T last_value)
{
    if (last == first ) {
        return false;
    }
    do {
        if (++(*(--last)) != last_value) {
            return true;
        }
        *last = first_value;
    } while (last != first);
    return false;
}

template <class BidirectionalIterator, class T, class Incrementer>
  bool
  next_mapping(BidirectionalIterator first,
               BidirectionalIterator last, 
               T first_value, T last_value, Incrementer increment)
{
    if (last == first ) {
        return false;
    }
    do {
        if (incrementer(*(--last)) != last_value) {
            return true;
        }
        *last = first_value;
    } while (last != first);
    return false;
}

/* PROPOSED STANDARD EXTENSION:
 *
 * template<class BidirectionalIterator, class T> 
 *   bool prev_mapping(BidirectionalIterator first,
 *                     BidirectionalIterator last,
 *                     T first_value, T last_value); 
 *
 * template<class BidirectionalIterator, class T, class Decrementor> 
 *   bool prev_mapping(BidirectionalIterator first,
 *                     BidirectionalIterator last,
 *                     T first_value, T last_value, Decrementor decrement); 
 */

template <class BidirectionalIterator, class T>
  bool
  prev_mapping(BidirectionalIterator first,
    	       BidirectionalIterator last,
               T first_value, T last_value)
{
    if (last == first) {
        return false;
    }
    --last_value;
    do {
        if (*(--last) != first_value) {
            --(*last);
            return true;
        }
        *last = last_value;
    } while (last != first);
    return true;
}

template <class BidirectionalIterator, class T, class Decrementer>
  bool
  prev_mapping(BidirectionalIterator first,
               BidirectionalIterator last, 
               T first_value, T last_value, Decrementer decrement)
{
    if (last == first) {
        return false;
    }
    decrement(last_value);
    do {
        if (*(--last) != first_value) {
            decrement(*last);
            return true;
        }
        *last = last_value;
    } while (last != first);
    return true;
}

/* PROPOSED STANDARD EXTENSION:
 *
 * template<class BidirectionalIterator, class T> 
 *   bool next_combination_counts(BidirectionalIterator first,
 *                                BidirectionalIterator last);
 */
 
template <class BidirectionalIterator>
  bool
  next_combination_counts(BidirectionalIterator first,
    	                 BidirectionalIterator last)
{
    BidirectionalIterator current = last;
    while (current != first && *(--current) == 0) {
    }
    if (current == first) {
        if (first != last && *first != 0)
            std::iter_swap(--last, first);
        return false;
    }
    --(*current);
    std::iter_swap(--last, current);
    ++(*(--current));
    return true;
}

/* PROPOSED STANDARD EXTENSION:
 *
 * template<class BidirectionalIterator> 
 *   bool prev_combination_counts(BidirectionalIterator first,
 *                                BidirectionalIterator last); 
 */

template <class BidirectionalIterator>
bool
prev_combination_counts(BidirectionalIterator first,
                        BidirectionalIterator last)
{
    if (first == last)
        return false;
    BidirectionalIterator current = --last;
    while (current != first && *(--current) == 0) {
    }
    if (current == last || ( current == first && *current == 0 )) {
        if (first != last)
            std::iter_swap(first, last);
        return false; 
    }
    --(*current);
    ++current;
    if (0 != *last) {
        std::iter_swap(current, last);
    }
    ++(*current);
    return true;
}

} // namespace boost

#endif // BOOST_ALGORITHM_COMBINATION_HPP
