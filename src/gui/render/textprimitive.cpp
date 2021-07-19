// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

// This include must come before the others
#include "TextPrimitiveLexer.h"

#include "base/sysfunc.h"
#include "gui/render/TextPrimitiveErrorListeners.h"
#include "gui/render/TextPrimitiveVisitor.h"
#include "gui/render/fontinstance.h"
#include "gui/render/symbol.h"
#include "gui/render/textprimitive.h"
#include <QtGui/qopengl.h>

// Return enum options for TextAnchor
EnumOptions<TextPrimitive::TextAnchor> TextPrimitive::textAnchors()
{
    return EnumOptions<TextPrimitive::TextAnchor>("TextAnchor", {{TextPrimitive::TopLeftAnchor, "TopLeft"},
                                                                 {TextPrimitive::TopMiddleAnchor, "TopMiddle"},
                                                                 {TextPrimitive::TopRightAnchor, "TopRight"},
                                                                 {TextPrimitive::MiddleLeftAnchor, "MiddleLeft"},
                                                                 {TextPrimitive::CentralAnchor, "Central"},
                                                                 {TextPrimitive::MiddleRightAnchor, "MiddleRight"},
                                                                 {TextPrimitive::BottomLeftAnchor, "BottomLeft"},
                                                                 {TextPrimitive::BottomMiddleAnchor, "BottomMiddle"},
                                                                 {TextPrimitive::BottomRightAnchor, "BottomRight"}});
}

// Set data
bool TextPrimitive::set(FontInstance *fontInstance, QString text, Vec3<double> anchorPoint,
                        TextPrimitive::TextAnchor anchorPosition, Vec3<double> adjustmentVector, Matrix4 localRotation,
                        double textSize, bool flat)
{
    // Create string stream and set up ANTLR input strem
    std::stringstream stream;
    stream << qPrintable(text);
    antlr4::ANTLRInputStream input(stream);

    // Create ANTLR lexer and set-up error listener
    TextPrimitiveLexer lexer(&input);
    TextPrimitiveLexerErrorListener lexerErrorListener(qPrintable(text));
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lexerErrorListener);

    // Generate tokens from input stream
    antlr4::CommonTokenStream tokens(&lexer);

    // Create ANTLR parser and set-up error listenres
    TextPrimitiveParser parser(&tokens);
    TextPrimitiveParserErrorListener parserErrorListener(qPrintable(text));
    parser.removeErrorListeners();
    parser.removeParseListeners();
    parser.addErrorListener(&lexerErrorListener);
    parser.addErrorListener(&parserErrorListener);

    // Generate the AST
    TextPrimitiveParser::PrimitiveContext *tree = nullptr;
    try
    {
        tree = parser.primitive();
    }
    catch (TextPrimitiveExceptions::TextPrimitiveSyntaxException &ex)
    {
        return Messenger::error(ex.what());
    };

    // Visit the nodes in the AST
    TextPrimitiveVisitor visitor;
    try
    {
        fragments_ = visitor.create(tree, fontInstance);
    }
    catch (TextPrimitiveExceptions::TextPrimitiveSyntaxException &ex)
    {
        return Messenger::error(ex.what());
    }

    anchorPoint_ = anchorPoint;
    anchorPosition_ = anchorPosition;
    adjustmentVector_ = adjustmentVector;
    localRotation_ = localRotation;
    textSize_ = textSize;
    flat_ = flat;

    return true;
}

// Return transformation matrix to use when rendering the text
Matrix4 TextPrimitive::transformationMatrix(FontInstance &fontInstance, const Matrix4 &viewMatrixInverse, double baseFontSize,
                                            OptionalReferenceWrapper<const TextFragment> optFragment) const
{
    Matrix4 textMatrix, A;
    Vec3<double> lowerLeft, upperRight, anchorPos, anchorPosRotated, textCentre;

    // Calculate scaling factor for font
    double scale = fontInstance.fontBaseHeight() * textSize_ / baseFontSize;

    // Calculate bounding box and anchor position on it
    boundingBox(fontInstance, lowerLeft, upperRight);
    switch (anchorPosition_)
    {
        case (TextPrimitive::TopLeftAnchor):
            anchorPos.set(lowerLeft.x, upperRight.y, 0.0);
            break;
        case (TextPrimitive::TopMiddleAnchor):
            anchorPos.set((lowerLeft.x + upperRight.x) * 0.5, upperRight.y, 0.0);
            break;
        case (TextPrimitive::TopRightAnchor):
            anchorPos = upperRight;
            break;
        case (TextPrimitive::MiddleLeftAnchor):
            anchorPos.set(lowerLeft.x, (lowerLeft.y + upperRight.y) * 0.5, 0.0);
            break;
        case (TextPrimitive::CentralAnchor):
            anchorPos.set((lowerLeft.x + upperRight.x) * 0.5, (lowerLeft.y + upperRight.y) * 0.5, 0.0);
            break;
        case (TextPrimitive::MiddleRightAnchor):
            anchorPos.set(upperRight.x, (lowerLeft.y + upperRight.y) * 0.5, 0.0);
            break;
        case (TextPrimitive::BottomLeftAnchor):
            anchorPos = lowerLeft;
            break;
        case (TextPrimitive::BottomMiddleAnchor):
            anchorPos.set((lowerLeft.x + upperRight.x) * 0.5, lowerLeft.y, 0.0);
            break;
        case (TextPrimitive::BottomRightAnchor):
            anchorPos.set(upperRight.x, lowerLeft.y, 0.0);
            break;
        default:
            break;
    }

    // Rotate anchor position with local rotation matrix
    textCentre = (lowerLeft + upperRight) * 0.5;
    anchorPosRotated = localRotation_ * (anchorPos - textCentre) * scale;

    // Construct matrix

    // -- Translate to centre of text bounding box (not rotated) accounting for fragment translation if one was specified
    if (optFragment)
        textCentre -= optFragment->get().translation();
    textMatrix.createTranslation(-textCentre);

    // -- Apply scaled local rotation matrix (if not flat)
    if (flat_)
        A = viewMatrixInverse;
    else
        A = localRotation_;
    A.applyScaling(scale, scale, scale);
    textMatrix.preMultiply(A);

    // -- Apply translation to text anchor point
    textMatrix.applyPreTranslation(-anchorPosRotated + anchorPoint_ + adjustmentVector_ * scale);

    // -- Apply fragment specific operations
    if (optFragment)
    {
        // -- Apply local scaling to text (if fragment was provided)
        textMatrix.applyScaling(optFragment->get().scale());
        // -- Apply local shear to text (if fragment is italic)
        if (optFragment->get().isItalic())
            textMatrix.applyShearX(0.2);
    }

    return textMatrix;
}

// Calculate bounding box of primitive
void TextPrimitive::boundingBox(FontInstance &fontInstance, Vec3<double> &lowerLeft, Vec3<double> &upperRight) const
{
    // Check for zero fragments
    if (fragments_.empty())
    {
        lowerLeft.zero();
        upperRight.zero();
        return;
    }

    // Loop over remaining fragments, keeping track of the total width of the primitive and the max/min y values
    Vec3<double> ll, ur;
    auto firstFragment = true;
    for (const auto &fragment : fragments_)
    {
        // Get bounding box for this fragment
        fontInstance.boundingBox(fragment.text(), ll, ur);

        // Scale the box by the current scaling factor...
        ur.x = ll.x + (ur.x - ll.x) * fragment.scale();
        ur.y = ll.y + (ur.y - ll.y) * fragment.scale();

        // Translate the box by the defined amount
        ll += fragment.translation();
        ur += fragment.translation();

        // Update lowerLeft and upperRight values
        if (firstFragment)
        {
            lowerLeft = ll;
            upperRight = ur;
            firstFragment = false;
        }
        else
        {
            if (ll.y < lowerLeft.y)
                lowerLeft.y = ll.y;
            if (ur.y > upperRight.y)
                upperRight.y = ur.y;
            if (ur.x > upperRight.x)
                upperRight.x = ur.x;
        }
    }
}

// Render primitive
void TextPrimitive::render(FontInstance &fontInstance, const Matrix4 &viewMatrix, const Matrix4 &viewMatrixInverse,
                           double baseFontSize) const
{
    Matrix4 textMatrix;

    // Loop over fragments
    for (const auto &fragment : fragments_)
    {
        textMatrix = viewMatrix * transformationMatrix(fontInstance, viewMatrixInverse, baseFontSize, fragment);
        glLoadMatrixd(textMatrix.matrix());

        // Draw bounding boxes around each fragment
        if (false)
        {
            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
            Vec3<double> ll, ur;
            fontInstance.boundingBox(fragment.text(), ll, ur);
            glBegin(GL_LINE_LOOP);
            glVertex3d(ll.x, ll.y, 0.0);
            glVertex3d(ur.x, ll.y, 0.0);
            glVertex3d(ur.x, ur.y, 0.0);
            glVertex3d(ll.x, ur.y, 0.0);
            glEnd();
        }

        // Apply any scaling stored in the FontInstance
        glScaled(fontInstance.scaleFactor(), fontInstance.scaleFactor(), fontInstance.scaleFactor());

        if (fragment.isBold())
        {
            // Render the text twice - once with lines, and once with polygon fill
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            fontInstance.renderText(fragment.text());
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            fontInstance.renderText(fragment.text());
        }
        else
            fontInstance.renderText(fragment.text());
    }
}
