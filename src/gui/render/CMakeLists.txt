# TextPrimitive ANTLR Lexer/Parser
antlr_target(TextPrimitiveGrammarLexer TextPrimitiveLexer.g4 LEXER)
# PACKAGE TextPrimitiveGrammar)
antlr_target(
  TextPrimitiveGrammarParser
  TextPrimitiveParser.g4
  PARSER
  # PACKAGE TextPrimitiveGrammar
  DEPENDS_ANTLR
  TextPrimitiveGrammarLexer
  COMPILE_FLAGS
  -no-listener
  -visitor
  -lib
  ${ANTLR_TextPrimitiveGrammarLexer_OUTPUT_DIR}
)

# Append path to ANTLR parser output, and set cache variable
list(APPEND ANTLR_OUTPUT_DIRS ${ANTLR_TextPrimitiveGrammarLexer_OUTPUT_DIR})
list(APPEND ANTLR_OUTPUT_DIRS ${ANTLR_TextPrimitiveGrammarParser_OUTPUT_DIR})
set(ANTLR_OUTPUT_DIRS
    ${ANTLR_OUTPUT_DIRS}
    CACHE INTERNAL ""
)

# Target 'render'
add_library(
  render
  axes.h
  colourdefinition.h
  colourscaledelta.h
  colourscale.h
  colourscalepoint.h
  fontinstance.h
  linestipple.h
  linestyle.h
  numberformat.h
  primitiveassembly.h
  primitive.h
  primitiveinfo.h
  primitiveinstance.h
  primitivelist.h
  renderable.h
  renderableconfiguration.h
  renderabledata1d.h
  renderabledata2d.h
  renderabledata3d.h
  renderablegroup.h
  renderablegroupmanager.h
  renderablespecies.h
  renderablespeciessite.h
  symbol.h
  textformat.h
  textfragment.h
  textprimitive.h
  textprimitivelist.h
  vertexchunk.h
  view.h
  TextPrimitiveErrorListeners.h
  TextPrimitiveVisitor.h
  axes.cpp
  colourdefinition.cpp
  colourscale.cpp
  colourscaledelta.cpp
  colourscalepoint.cpp
  fontinstance.cpp
  linestipple.cpp
  linestyle.cpp
  numberformat.cpp
  primitiveassembly.cpp
  primitive.cpp
  primitiveinfo.cpp
  primitiveinstance.cpp
  primitivelist.cpp
  renderable.cpp
  renderableconfiguration.cpp
  renderabledata1d.cpp
  renderabledata2d.cpp
  renderabledata3d.cpp
  renderablegroup.cpp
  renderablegroupmanager.cpp
  renderablespecies.cpp
  renderablespeciessite.cpp
  symbol.cpp
  textformat.cpp
  textfragment.cpp
  textprimitive.cpp
  textprimitivelist.cpp
  vertexchunk.cpp
  view.cpp
  TextPrimitiveErrorListeners.cpp
  TextPrimitiveVisitor.cpp
  ${ANTLR_TextPrimitiveGrammarLexer_CXX_OUTPUTS}
  ${ANTLR_TextPrimitiveGrammarParser_CXX_OUTPUTS}
)

target_include_directories(
  render
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${PROJECT_BINARY_DIR}/src
          ${Qt6Widgets_INCLUDE_DIRS}
          ${FREETYPE_INCLUDE_DIRS}
          ${ANTLRRUNTIME_INCLUDE_DIRS}
          ${ANTLR_TextPrimitiveGrammarLexer_OUTPUT_DIR}
          ${ANTLR_TextPrimitiveGrammarParser_OUTPUT_DIR}
)

target_link_libraries(render PRIVATE base)
