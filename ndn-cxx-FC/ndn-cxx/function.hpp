/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2017 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Jeff Thompson <jefft0@remap.ucla.edu>
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 * @author Zhenkai Zhu <http://irl.cs.ucla.edu/~zhenkai/>
 */

#ifndef NDN_CXX_FUNCTION_HPP
#define NDN_CXX_FUNCTION_HPP

#include "name-component.hpp"
#include <iterator>

namespace ndn {

class Function;

/** @brief Represents an arbitrary sequence of name components
 */
using PartialFunction = Function;

/** @brief Represents an absolute name
 */
class Function
{
public: // nested types
  class Error : public name::Component::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : name::Component::Error(what)
    {
    }
  };

  using Component = name::Component;
  using component_container = std::vector<Component>;

  // Function appears as a container of name components
  using value_type             = Component;
  using allocator_type         = void;
  using reference              = Component&;
  using const_reference        = const Component&;
  using pointer                = Component*;
  using const_pointer          = const Component*;
  using iterator               = const Component*; // disallow modifying via iterator
  using const_iterator         = const Component*;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using difference_type        = component_container::difference_type;
  using size_type              = component_container::size_type;

public: // constructors, encoding, decoding
  /** @brief Create an empty name
   *  @post empty() == true
   */
  Function();

  /** @brief Decode Function from wire encoding
   *  @throw tlv::Error wire encoding is invalid
   *
   *  This is a more efficient equivalent for
   *  @code
   *    Function name;
   *    name.wireDecode(wire);
   *  @endcode
   */
  explicit
  Function(const Block& wire);

  /** @brief Parse name from NDN URI
   *  @param uri a null-terminated URI string
   *  @sa https://named-data.net/doc/ndn-tlv/name.html#ndn-uri-scheme
   */
  Function(const char* uri);

  /** @brief Create name from NDN URI
   *  @param uri a URI string
   *  @sa https://named-data.net/doc/ndn-tlv/name.html#ndn-uri-scheme
   */
  Function(std::string uri);

  /** @brief Get URI representation of the name
   *  @return URI representation; "ndn:" scheme identifier is not included
   *  @sa https://named-data.net/doc/ndn-tlv/name.html#ndn-uri-scheme
   *  @note To print URI representation into a stream, it is more efficient to use ``os << name``.
   */
  std::string
  toUri() const;

  /** @brief Check if this Function instance already has wire encoding
   */
  bool
  hasWire() const
  {
    return m_wire.hasWire();
  }

  /** @brief Fast encoding or block size estimation
   */
  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  /** @brief Perform wire encoding, or return existing wire encoding
   *  @post hasWire() == true
   */
  const Block&
  wireEncode() const;

  /** @brief Decode name from wire encoding
   *  @throw tlv::Error wire encoding is invalid
   *  @post hasWire() == true
   */
  void
  wireDecode(const Block& wire);

  /** @brief Make a deep copy of the name, reallocating the underlying memory buffer
   */
  Function
  deepCopy() const;

public: // access
  /** @brief Check if name is empty
   */
  bool
  empty() const
  {
    return m_wire.elements().empty();
  }

  /** @brief Get number of components
   */
  size_t
  size() const
  {
    return m_wire.elements_size();
  }

  /** @brief Get the component at the given index
   *  @param i zero-based index; if negative, it starts at the end of this name
   *  @warning Indexing out of bounds triggers undefined behavior.
   */
  const Component&
  get(ssize_t i) const
  {
    if (i < 0) {
      i += size();
    }
    return reinterpret_cast<const Component&>(m_wire.elements()[i]);
  }

  /** @brief Equivalent to get(i)
   */
  const Component&
  operator[](ssize_t i) const
  {
    return get(i);
  }

  /** @brief Get the component at the given index
   *  @param i zero-based index; if negative, size()+i is used instead
   *  @throws Function::Error index is out of bounds
   */
  const Component&
  at(ssize_t i) const;

  /** @brief Extract some components as a sub-name (PartialFunction)
   *  @param iStartComponent zero-based index of the first component;
   *                         if negative, size()+iStartComponent is used instead
   *  @param nComponents Number of components starting at iStartComponent.
   *                     Use @p npos to get the PartialFunction until the end of this Function.
   *  @return a new PartialFunction containing the extracted components
   *
   *  If iStartComponent is positive and indexes out of bounds, returns an empty PartialFunction.
   *  If iStartComponent is negative and indexes out of bounds, returns components starting from the
   *  beginning of the Function. If nComponents is out of bounds, returns the components until the end
   *  of this Function.
   */
  PartialFunction
  getSubFunction(ssize_t iStartComponent, size_t nComponents = npos) const;

  /** @brief Extract a prefix of the name
   *  @param nComponents Number of components; if negative, size()+nComponents is used instead
   *  @return a new Function containing the prefix
   *                    the prefix up to name.size() - N. For example getPrefix(-1)
   *                    returns the name without the final component.
   * @return A new partial name
   */
  PartialFunction
  getPrefix(ssize_t nComponents) const
  {
    if (nComponents < 0)
      return getSubFunction(0, size() + nComponents);
    else
      return getSubFunction(0, nComponents);
  }

public: // iterators
  /** @brief Begin iterator
   */
  const_iterator
  begin() const
  {
    return reinterpret_cast<const_iterator>(m_wire.elements().data());
  }

  /** @brief End iterator
   */
  const_iterator
  end() const
  {
    return reinterpret_cast<const_iterator>(m_wire.elements().data() + m_wire.elements().size());
  }

  /** @brief Reverse begin iterator
   */
  const_reverse_iterator
  rbegin() const
  {
    return const_reverse_iterator(end());
  }

  /** @brief Reverse end iterator
   */
  const_reverse_iterator
  rend() const
  {
    return const_reverse_iterator(begin());
  }

public: // modifiers
  /** @brief Append a component
   *  @return a reference to this name, to allow chaining
   */
  Function&
  append(const Component& component)
  {
    m_wire.push_back(component);
    return *this;
  }

  /** @brief Append a component, copying from a zero-terminated byte string
   *  @param value a zero-terminated string; it will be constructed as a component as is
   *               and will not be interpreted as a URI
   *  @note This overload is necessary to ensure unambiguity of ``append("string")``, because both
   *        Component and PartialFunction are implicitly convertible from zero-terminated byte string.
   *        This overload ensures the string is constructed as a Component.
   */
  Function&
  append(const char* value)
  {
    return append(Component(value));
  }

  /** @brief Append a component, copying from [@p value, @p value + @p valueLength)
   *  @return a reference to this name, to allow chaining
   */
  Function&
  append(const uint8_t* value, size_t valueLength)
  {
    return append(Component(value, valueLength));
  }

  /** @brief Append a component, copying from [@p first, @p last)
   *  @tparam Iterator an InputIterator dereferencing to a one-octet value type. More efficient
   *                   implementation is available when @p Iterator additionally satisfies
   *                   RandomAccessIterator concept.
   *  @param first     begin position of the value
   *  @param last      end position of the value
   *  @return a reference to this name, to allow chaining
   */
  template<class Iterator>
  Function&
  append(Iterator first, Iterator last)
  {
    static_assert(sizeof(typename std::iterator_traits<Iterator>::value_type) == 1,
                  "iterator does not dereference to one-octet value type");
    return append(Component(first, last));
  }

  /** @brief Append a component, decoding from a Block
   *  @param value a Block; if its TLV-TYPE is not FunctionComponent, it is nested into a FunctionComponent
   *  @return a reference to this name, to allow chaining
   */
  Function&
  append(const Block& value)
  {
    if (value.type() == tlv::GenericNameComponent) {
      m_wire.push_back(value);
    }
    else {
      m_wire.push_back(Block(tlv::GenericNameComponent, value));
    }

    return *this;
  }

  /** @brief Append a component with a nonNegativeInteger
   *  @sa number the number
   *  @return a reference to this name, to allow chaining
   *  @sa https://named-data.net/doc/ndn-tlv/tlv.html#non-negative-integer-encoding
   */
  Function&
  appendNumber(uint64_t number)
  {
    return append(Component::fromNumber(number));
  }

  /** @brief Append a component with a marked number
   *  @param marker 1-octet marker
   *  @param number the number
   *
   *  The component is encoded as a 1-octet marker, followed by a nonNegativeInteger.
   *
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendNumberWithMarker(uint8_t marker, uint64_t number)
  {
    return append(Component::fromNumberWithMarker(marker, number));
  }

  /** @brief Append a version component
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendVersion(uint64_t version)
  {
    return append(Component::fromVersion(version));
  }

  /** @brief Append a version component based on current time
   *
   *  The version number is the current UNIX timestamp in milliseconds
   *
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendVersion();

  /** @brief Append a segment number (sequential) component
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendSegment(uint64_t segmentNo)
  {
    return append(Component::fromSegment(segmentNo));
  }

  /** @brief Append a segment byte offset component
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendByteOffset(uint64_t offset)
  {
    return append(Component::fromByteOffset(offset));
  }

  /** @brief Append a timestamp component
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendTimestamp(const time::system_clock::TimePoint& timePoint)
  {
    return append(Component::fromTimestamp(timePoint));
  }

  /** @brief Append a timestamp component based on current time
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendTimestamp();

  /** @brief Append a sequence number component
   *  @return a reference to this name, to allow chaining
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Function&
  appendSequenceNumber(uint64_t seqNo)
  {
    return append(Component::fromSequenceNumber(seqNo));
  }

  /** @brief Append an ImplicitSha256Digest component
   *  @return a reference to this name, to allow chaining
   */
  Function&
  appendImplicitSha256Digest(const ConstBufferPtr& digest)
  {
    return append(Component::fromImplicitSha256Digest(digest));
  }

  /** @brief Append an ImplicitSha256Digest component
   *  @return a reference to this name, to allow chaining
   */
  Function&
  appendImplicitSha256Digest(const uint8_t* digest, size_t digestSize)
  {
    return append(Component::fromImplicitSha256Digest(digest, digestSize));
  }

  /** @brief Append a PartialFunction
   *  @param name the components to append
   *  @return a reference to this name, to allow chaining
   */
  Function&
  append(const PartialFunction& name);

  /** @brief Append a component
   *  @note This makes push_back an alias of append, giving Function a similar API as STL vector.
   */
  template<class T>
  void
  push_back(const T& component)
  {
    append(component);
  }

  /** @brief Remove all components
   *  @post empty() == true
   */
  void
  clear()
  {
    m_wire = Block(tlv::Function);
  }

public: // algorithms
  /** @brief Get the successor of a name
   *
   *  The successor of a name is defined as follows:
   *
   *      N represents the set of NDN Functions, and X,Y ∈ N.
   *      Operator < is defined by canonical order on N.
   *      Y is the successor of X, if (a) X < Y, and (b) ∄ Z ∈ N s.t. X < Z < Y.
   *
   *  In plain words, successor of a name is the same name, but with its last component
   *  advanced to a next possible value.
   *
   *  Examples:
   *
   *  - successor for / is /%00
   *  - successor for /%00%01/%01%02 is /%00%01/%01%03
   *  - successor for /%00%01/%01%FF is /%00%01/%02%00
   *  - successor for /%00%01/%FF%FF is /%00%01/%00%00%00
   *
   *  @return a new Function containing the successor
   */
  Function
  getSuccessor() const;

  /** @brief Check if this name is a prefix of another name
   *
   *  This name is a prefix of @p other if the N components of this name are same as the first N
   *  components of @p other.
   *
   *  @retval true this name is a prefix of @p other
   *  @retval false this name is not a prefix of @p other
   */
  bool
  isPrefixOf(const Function& other) const;

  /** @brief Check if this name equals another name
   *
   *  Two names are equal if they have the same number of components, and components at each index
   *  are equal.
   */
  bool
  equals(const Function& other) const;

  /** @brief Compare this to the other Function using NDN canonical ordering.
   *
   *  If the first components of each name are not equal, this returns a negative value if
   *  the first comes before the second using the NDN canonical ordering for name
   *  components, or a positive value if it comes after.  If they are equal, this compares
   *  the second components of each name, etc. If both names are the same up to the size
   *  of the shorter name, this returns a negative value if the first name is shorter than
   *  the second or a positive value if it is longer.  For example, if you std::sort gives:
   *  /a/b/d /a/b/cc /c /c/a /bb .
   *  This is intuitive because all names with the prefix /a are next to each other.
   *  But it may be also be counter-intuitive because /c comes before /bb according
   *  to NDN canonical ordering since it is shorter.
   *
   *  @param other The other Function to compare with.
   *
   *  @retval negative this comes before other in canonical ordering
   *  @retval zero this equals other
   *  @retval positive this comes after other in canonical ordering
   *
   *  @sa https://named-data.net/doc/ndn-tlv/name.html#canonical-order
   */
  int
  compare(const Function& other) const
  {
    return this->compare(0, npos, other);
  }

  /** @brief compares [pos1, pos1+count1) components in this Function
   *         to [pos2, pos2+count2) components in @p other
   *
   *  This is equivalent to this->getSubFunction(pos1, count1).compare(other.getSubFunction(pos2, count2));
   */
  int
  compare(size_t pos1, size_t count1,
          const Function& other, size_t pos2 = 0, size_t count2 = npos) const;

public:
  /** @brief indicates "until the end" in getSubFunction and compare
   */
  static const size_t npos;

private:
  mutable Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Function);

inline bool
operator==(const Function& lhs, const Function& rhs)
{
  return lhs.equals(rhs);
}

inline bool
operator!=(const Function& lhs, const Function& rhs)
{
  return !lhs.equals(rhs);
}

inline bool
operator<=(const Function& lhs, const Function& rhs)
{
  return lhs.compare(rhs) <= 0;
}

inline bool
operator<(const Function& lhs, const Function& rhs)
{
  return lhs.compare(rhs) < 0;
}

inline bool
operator>=(const Function& lhs, const Function& rhs)
{
  return lhs.compare(rhs) >= 0;
}

inline bool
operator>(const Function& lhs, const Function& rhs)
{
  return lhs.compare(rhs) > 0;
}

/** @brief Print URI representation of a name
 *  @sa https://named-data.net/doc/ndn-tlv/name.html#ndn-uri-scheme
 */
std::ostream&
operator<<(std::ostream& os, const Function& name);

/** @brief Parse URI from stream as Function
 *  @sa https://named-data.net/doc/ndn-tlv/name.html#ndn-uri-scheme
 */
std::istream&
operator>>(std::istream& is, Function& name);

} // namespace ndn

namespace std {

template<>
struct hash<ndn::Function>
{
  size_t
  operator()(const ndn::Function& name) const;
};

} // namespace std

#endif // NDN_CXX_FUNCTION_HPP
