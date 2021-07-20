---
title: Add (Node)
linkTitle: Add
description: Insert molecules into a box
---

{{< htable >}}
| | |
|--------|----------|
|Context|Generation|
|Name Required?|No|
|Branches|--|
{{< /htable >}}

## Overview

The `Add` node is a core component of nearly all configuration generators requiring the initial construction of a suitable starting point. It adds a number of copies of a given target species to the current box, either at specific or random coordinates. By default, the box is resized to accommodate the new population of molecules based on a supplied density.

If the species is periodic, the box associated with that species can be copied to the model as part of the `Add` process. In that case, an initial [`Box`]({{< ref boxnode >}}) node is not necessary in the procedure.

## Description

When adding molecules of the target species to the current configuration, there are several possible actions on the periodic box for that configuration:

1. No resizing of the box is performed ([`BoxAction`]({{< ref "boxactionstyle" >}}) = `None`) - The box retains its original geometry regardless of the number of added molecules. This is useful if an explicit box geometry has already been set by a [`Box`]({{< ref "boxnode" >}}) node, or it has existing contents that should not be resized (e.g. a crystal).
2. The box volume is increased to accommodate the new molecules ([`BoxAction`]({{< ref "boxactionstyle" >}}) = `AddVolume`) - Additional volume is added to the box based on the density and population of the target species. Existing molecules within the box have their centres of geometry scaled as well.
3. The box volume is scaled to give the supplied density ([`BoxAction`]({{< ref "boxactionstyle" >}}) = `ScaleVolume`) - The box volume is scaled to give the requested density, taking into account the existing box contents and the new population of molecules requested. Existing molecules within the box have their centres of geometry scaled as well.
4. The box is set ([`BoxAction`]({{< ref "boxactionstyle" >}}) = `Set`) from a definition on the species itself (if it is periodic). This is typically used when the species is a framework-type model (e.g. a metal organic framework) and it is to be used as the main basis for a configuration.

When resizing the box, the default is to scale equally in all directions A, B, and C. The scaling can be restricted to two or even one principal axis by setting `ScaleA`, `ScaleB`, and/or `ScaleC` to `false`. At least one axis must remain scalable, for obvious reasons!

## Configuration

### Control Keywords

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`BoxAction`|[`BoxActionStyle`]({{< ref "boxactionstyle" >}})|`AddVolume`|Action to take on the Box geometry / volume on addition of the species|
|`Density`|[`expr`]({{< ref "expressions" >}})<br/>[`DensityUnit`]({{< ref "densityunit" >}})|`0.1 atoms/A3`|Density at which to add the target species. Note that the use of this value differs according to the selected `BoxAction` (see above).|
|`Population`|[`expr`]({{< ref "expressions" >}})|`0`|Population of the target species to add.|
|`Positioning`|[`PositioningType`]({{< ref "positioningtype" >}})|`Random`|Positioning type for individual molecules.|
|`Region`|`name`|--|Region node controlling the location of inserted species into the configuration.|
|`Rotate`|`true|false`|`true`|Whether to randomly rotate molecules on insertion.|
|`ScaleA`|`true|false`|`true`|Whether to scale the A cell axis when changing the cell volume.|
|`ScaleB`|`true|false`|`true`|Whether to scale the B cell axis when changing the cell volume.|
|`ScaleC`|`true|false`|`true`|Whether to scale the C cell axis when changing the cell volume.|
|`Species`|`name`|--|{{< required-label >}} Target species to add.|
